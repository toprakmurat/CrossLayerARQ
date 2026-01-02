#include "PhysicalLayer.hpp"
#include "Common.hpp"
#include "LinkLayer.hpp"
#include <cmath>

namespace ARQ {

PhysicalLayer::PhysicalLayer(SimulatorEngine &engine, uint64_t bit_rate,
                             std::chrono::milliseconds propagation_delay)
    : engine_(engine), bit_rate_(bit_rate),
      propagation_delay_(propagation_delay), rng_(std::random_device{}()) {}

void PhysicalLayer::SetPeer(std::shared_ptr<PhysicalLayer> peer) {
  peer_phy_ = peer;
}

void PhysicalLayer::SetUpperLayer(std::shared_ptr<LinkLayer> upper) {
  upper_layer_ = upper;
}

void PhysicalLayer::Transmit(std::shared_ptr<const Frame> frame) {
  // Serialization Delay = SizeInBits / BitRate
  // Size = Header + Payload
  size_t total_bytes = sizeof(FrameHeader) + frame->payload.size();
  uint64_t total_bits = total_bytes * 8;

  // bit_rate_ is in bps.
  // delay in micros = (bits * 1e6) / rate
  uint64_t serialization_micros = (total_bits * 1'000'000) / bit_rate_;
  std::chrono::microseconds ser_delay(serialization_micros);

  // Propagation Delay
  // Total Delay = Ser + Prop
  SimTime arrival_delay = ser_delay + propagation_delay_;

  // Schedule Arrival at Peer
  // Note: Peer must exist (weak_ptr lock)
  if (auto peer = peer_phy_.lock()) {
    // Capture frame and peer by value/shared_ptr
    engine_.Schedule(arrival_delay,
                     [peer, frame]() { peer->OnFrameArrival(frame); });
  }
}

void PhysicalLayer::OnFrameArrival(std::shared_ptr<const Frame> frame) {
  UpdateChannelState();

  // Calculate total size including headers
  size_t total_bytes = sizeof(FrameHeader) + frame->payload.size();

  if (ShouldDrop(total_bytes)) {
    // Drop packet
    return;
  }

  // Processing Delay before handing to Upper Layer
  SimTime proc_delay = PROCESSING_DELAY();

  if (auto up = upper_layer_.lock()) {
    engine_.Schedule(proc_delay, [up, frame]() { up->Receive(frame); });
  }
}

void PhysicalLayer::UpdateChannelState() {
  double roll = dist_(rng_);
  if (state_ == ChannelState::GOOD) {
    if (roll < P_GOOD_TO_BAD()) {
      state_ = ChannelState::BAD;
    }
  } else {
    if (roll < P_BAD_TO_GOOD()) {
      state_ = ChannelState::GOOD;
    }
  }
}

bool PhysicalLayer::ShouldDrop(size_t packet_size_bytes) {
  double ber = (state_ == ChannelState::GOOD) ? BER_GOOD() : BER_BAD();
  // PER = 1 - (1 - BER)^bits
  double per = 1.0 - std::pow(1.0 - ber, packet_size_bytes * 8);
  return dist_(rng_) < per;
}

} // namespace ARQ
