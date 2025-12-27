#pragma once

#include "Common.hpp"
#include "Engine.hpp"
#include "Packets.hpp"
#include <memory>
#include <random>

namespace ARQ {

class LinkLayer;

class PhysicalLayer {
public:
  PhysicalLayer(SimulatorEngine &engine, uint64_t bit_rate,
                std::chrono::milliseconds propagation_delay);

  void SetPeer(std::shared_ptr<PhysicalLayer> peer);
  void SetUpperLayer(std::shared_ptr<LinkLayer> upper);

  // --- SENDER API ---
  void Transmit(std::shared_ptr<const Frame> frame);

  // --- RECEIVER API ---
  void OnFrameArrival(std::shared_ptr<const Frame> frame);

private:
  SimulatorEngine &engine_;
  uint64_t bit_rate_;
  std::chrono::milliseconds propagation_delay_;

  std::weak_ptr<PhysicalLayer> peer_phy_;
  std::weak_ptr<LinkLayer> upper_layer_;

  // Error Model State
  enum class ChannelState { GOOD, BAD };
  ChannelState state_ = ChannelState::GOOD;

  std::mt19937 rng_;
  std::uniform_real_distribution<double> dist_{0.0, 1.0};

  void UpdateChannelState();
  bool ShouldDrop(size_t packet_size_bytes);
};

} // namespace ARQ
