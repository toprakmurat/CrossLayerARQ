#pragma once

#include "Common.hpp"
#include "Engine.hpp"
#include "Node.hpp"
#include <iostream>
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

  void SetupTopology(uint32_t window_size, size_t segment_size);
};

} // namespace ARQ
