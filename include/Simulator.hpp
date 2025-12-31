#pragma once

#include "Channel.hpp"
#include "Common.hpp"
#include "Engine.hpp"
#include "Node.hpp"
#include <memory>

namespace ARQ {

struct SimulationResult {
  double goodput_bps;
  SimTime total_time;
  size_t total_bytes_received;
};

class Simulator {
public:
  Simulator();

  // Run a single simulation with given parameters
  SimulationResult Run(uint32_t window_size, size_t segment_size);

private:
  SimulatorEngine engine_;

  std::shared_ptr<Node> nodeA_;
  std::shared_ptr<Node> nodeB_;
  std::shared_ptr<Channel> channel_;

  void SetupTopology(uint32_t window_size, size_t segment_size);
};

} // namespace ARQ
