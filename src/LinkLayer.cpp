#include "LinkLayer.hpp"
#include "Memory.hpp"
#include "PhysicalLayer.hpp"
#include "TransportLayer.hpp"
#include <algorithm>
#include <iostream>

namespace ARQ {

LinkLayer::LinkLayer(SimulatorEngine &engine, uint32_t window_size)
    : engine_(engine), window_size_(window_size) {}

void LinkLayer::SetLowerLayer(std::shared_ptr<PhysicalLayer> phy) {
  phy_ = phy;
}

void LinkLayer::SetUpperLayer(std::shared_ptr<TransportLayer> transport) {
  transport_ = transport;
}

void LinkLayer::Send(std::pmr::vector<std::byte> transport_segment) {
  pending_queue_.push_back({std::move(transport_segment)});
  TrySendNext();
}

void LinkLayer::SetPaused(bool paused) {
  is_paused_ = paused;
  if (!is_paused_) {
    TrySendNext();
  }
}

void LinkLayer::Receive(std::shared_ptr<const Frame> frame_ptr) {
  if (frame_ptr->header.link_header.type == FrameType::ACK) {
    HandleAck(frame_ptr);
  } else {
    HandleData(frame_ptr);
  }
}

void LinkLayer::TrySendNext() {
  if (is_paused_)
    return;

  while (!pending_queue_.empty() && send_window_.size() < window_size_) {
    auto &item = pending_queue_.front();

    std::pmr::polymorphic_allocator<Frame> alloc(Memory::packets());
    auto frame = std::allocate_shared<Frame>(alloc, Memory::packets());

    frame->payload = std::move(item.payload);
    pending_queue_.pop_front();

    frame->header.link_header.type = FrameType::DATA;
    frame->header.link_header.seq_num = next_seq_num_;
    frame->header.link_header.ack_num = 0;

    // Push BEFORE transmitting safely
    send_window_.push_back({frame, false, 0});

    // Use the frame ptr from window to ensure lifetime
    auto win_frame = send_window_.back().frame;
    uint32_t seq = next_seq_num_;

    // Timeout: 2.5 * RTT (approx 200ms)
    SimTime timeout = std::chrono::milliseconds(200);

    uint64_t timer_id =
        engine_.Schedule(timeout, [weak_self = weak_from_this(), seq]() {
          if (auto self = weak_self.lock()) {
            self->HandleTimeout(seq);
          }
        });

    send_window_.back().timer_event_id = timer_id;

    phy_->Transmit(win_frame);

    next_seq_num_++;
  }
}

void LinkLayer::HandleAck(std::shared_ptr<const Frame> frame) {
  uint32_t ack = frame->header.link_header.ack_num;

  // Selective Repeat ACK Logic
  // If ACK is in our window, mark it.
  if (ack >= send_base_) {
    size_t idx = ack - send_base_;
    if (idx < send_window_.size()) {
      // Cancel Timer if not already acked
      if (!send_window_[idx].acked) {
        send_window_[idx].acked = true;
        engine_.Cancel(send_window_[idx].timer_event_id);
      }
    }
  }

  // Slide Window
  while (!send_window_.empty() && send_window_.front().acked) {
    send_window_.pop_front();
    send_base_++;
  }

  TrySendNext();
}

void LinkLayer::HandleData(std::shared_ptr<const Frame> frame) {
  SendAck(frame->header.link_header.seq_num);

  // Pass Up
  if (auto t = transport_.lock()) {
    std::pmr::vector<std::byte> payload_copy = frame->payload;
    t->ReceiveUp(frame->header.transport_header, std::move(payload_copy));
  }
}

void LinkLayer::HandleTimeout(uint32_t seq_num) {
  // Retransmit logic
  // Find packet in window
  if (seq_num >= send_base_) {
    size_t idx = seq_num - send_base_;
    if (idx < send_window_.size()) {
      auto &slot = send_window_[idx];
      if (!slot.acked) {
        // Retransmit
        phy_->Transmit(slot.frame);

        // Reschedule Timer
        SimTime timeout = std::chrono::milliseconds(200);
        uint64_t new_id = engine_.Schedule(
            timeout, [weak_self = weak_from_this(), seq_num]() {
              if (auto self = weak_self.lock()) {
                self->HandleTimeout(seq_num);
              }
            });
        slot.timer_event_id = new_id;
      }
    }
  }
}

void LinkLayer::SendAck(uint32_t ack_num) {
  std::pmr::polymorphic_allocator<Frame> alloc(Memory::packets());
  auto frame = std::allocate_shared<Frame>(alloc, Memory::packets());

  frame->header.link_header.type = FrameType::ACK;
  frame->header.link_header.ack_num = ack_num;

  phy_->Transmit(frame);
}

} // namespace ARQ
