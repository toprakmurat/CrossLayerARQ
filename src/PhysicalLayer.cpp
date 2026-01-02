#include "PhysicalLayer.hpp"
#include "Common.hpp"
#include "Channel.hpp"
#include "LinkLayer.hpp"
#include <cmath>
#include <iostream>

namespace ARQ {

PhysicalLayer::PhysicalLayer(SimulatorEngine &engine, uint64_t bit_rate,
                             std::chrono::milliseconds propagation_delay)
    : engine_(engine), bit_rate_(bit_rate),
      propagation_delay_(propagation_delay) {}

void PhysicalLayer::SetPeer(std::shared_ptr<PhysicalLayer> peer) {
  peer_phy_ = peer;
}

void PhysicalLayer::SetUpperLayer(std::shared_ptr<LinkLayer> upper) {
  upper_layer_ = upper;
}

void PhysicalLayer::SetChannel(std::shared_ptr<Channel> channel) {
  channel_ = channel;
}

void PhysicalLayer::Transmit(std::shared_ptr<const Frame> frame) {
  std::cout << "[PhysicalLayer] Transmitting frame " << frame->header.link_header.seq_num
            << std::endl;

  // Serialization Delay = SizeInBits / BitRate
  // Size = Header + Payload
  size_t total_bytes = sizeof(FrameHeader) + frame->payload.size();
  uint64_t total_bits = total_bytes * 8;

  // bit_rate_ is in bps.
  // delay in micros = (bits * 1e6) / rate
  uint64_t serialization_micros = (total_bits * 1'000'000) / bit_rate_;
  std::chrono::microseconds ser_delay(serialization_micros);

  // Get propagation delay from channel
  auto prop_delay = channel_->GetPropagationDelay(shared_from_this());

  // Total Delay = Ser + Prop
  SimTime total_delay = ser_delay + prop_delay;

  // Transmit through channel (channel will add errors and deliver to receiver)
  channel_->Transmit(shared_from_this(), frame, total_delay);
}

void PhysicalLayer::OnFrameArrival(std::shared_ptr<const Frame> frame) {
  std::cout << "[PhysicalLayer] Frame arrived " << frame->header.link_header.seq_num
            << std::endl;

  // Processing Delay before handing to Upper Layer
  SimTime proc_delay = PROCESSING_DELAY();

  if (auto up = upper_layer_.lock()) {
    engine_.Schedule(proc_delay, [up, frame]() { up->Receive(frame); });
  }
}

} // namespace ARQ
