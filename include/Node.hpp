#pragma once

#include "ApplicationLayer.hpp"
#include "Common.hpp"
#include "Engine.hpp"
#include "LinkLayer.hpp"
#include "PhysicalLayer.hpp"
#include "TransportLayer.hpp"
#include <memory>

namespace ARQ {

struct NodeConfig {
  uint32_t window_size;
  size_t segment_size;
  bool is_sender{false};
  uint64_t bit_rate{BIT_RATE_BPS()};
  std::chrono::milliseconds prop_delay{PROPAGATION_DELAY_FWD()};
};

class Node {
public:
  Node(SimulatorEngine &engine, const NodeConfig &config);

  // Getters for external connections/stats
  std::shared_ptr<PhysicalLayer> GetPhy() const { return phy_; }
  std::shared_ptr<ApplicationLayer> GetApp() const { return app_; }

  void ConnectPeer(std::shared_ptr<Node> peer);

private:
  std::shared_ptr<PhysicalLayer> phy_;
  std::shared_ptr<LinkLayer> link_;
  std::shared_ptr<TransportLayer> trans_;
  std::shared_ptr<ApplicationLayer> app_;
};

} // namespace ARQ
