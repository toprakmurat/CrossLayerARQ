#include "LinkLayer.hpp"
#include "Memory.hpp"
#include "PhysicalLayer.hpp"
#include "TransportLayer.hpp"
#include <iostream>

namespace ARQ {

LinkLayer::LinkLayer(SimulatorEngine &engine, uint32_t window_size)
    : engine_(engine), window_size_(window_size), rx_window_(window_size) {}

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
  uint32_t seq = frame->header.link_header.seq_num;

  // Check if frame is within the Receptive Window [rx_base, rx_base + W)
  if (seq >= rx_base_ && seq < rx_base_ + window_size_) {
    size_t idx = seq - rx_base_;

    // Map seq to window index [0, window_size)
    if (idx < rx_window_.size()) {
      if (!rx_window_[idx].received) {
        // Store the frame
        std::pmr::polymorphic_allocator<Frame> alloc(Memory::packets());
        auto stored_frame =
            std::allocate_shared<Frame>(alloc, Memory::packets());
        stored_frame->header = frame->header;
        stored_frame->payload = frame->payload;

        rx_window_[idx].frame = stored_frame;
        rx_window_[idx].received = true;
      }
      // Always ACK received packet in window
      SendAck(seq);
    }
  } else if (seq < rx_base_) {
    // Duplicate/Old packet, must re-ACK to move sender forward
    SendAck(seq);
  }

  // Try to deliver contiguous frames from rx_base_
  while (!rx_window_.empty() && rx_window_.front().received) {
    auto &slot = rx_window_.front();

    // Try to pass up
    if (auto t = transport_.lock()) {
      bool accepted = t->ReceiveUp(slot.frame->header.transport_header,
                                   slot.frame->payload);

      if (accepted) {
        // Slide window
        rx_window_.erase(rx_window_.begin());
        rx_window_.push_back(RxWindowSlot{});
        rx_base_++;
      } else {
        // TODO: Backpressure handling
        break;
      }
    } else {
      break;
    }
  }
}

void LinkLayer::HandleTimeout(uint32_t seq_num) {
  std::cout << "[LinkLayer] Timeout for seq_num " << seq_num << std::endl;

  // Retransmit logic
  if (seq_num >= send_base_) {
    size_t idx = seq_num - send_base_;
    if (idx < send_window_.size()) {
      auto &slot = send_window_[idx];
      if (!slot.acked) {
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
