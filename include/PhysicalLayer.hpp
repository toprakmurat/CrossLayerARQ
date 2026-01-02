#pragma once

#include "Engine.hpp"
#include "Packets.hpp"
#include <memory>
#include <random>

namespace ARQ {

class LinkLayer;
class Channel;

class PhysicalLayer : public std::enable_shared_from_this<PhysicalLayer> {
public:
  PhysicalLayer(SimulatorEngine &engine, uint64_t bit_rate,
                std::chrono::milliseconds propagation_delay);

  void SetPeer(std::shared_ptr<PhysicalLayer> peer);
  void SetUpperLayer(std::shared_ptr<LinkLayer> upper);
  void SetChannel(std::shared_ptr<Channel> channel);

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
  std::shared_ptr<Channel> channel_;
};

} // namespace ARQ
