#include "Simulator.hpp"
#include "Common.hpp"
#include <iostream>

namespace ARQ {

Simulator::Simulator() {
  // Engine initialized
}

SimulationResult Simulator::Run(uint32_t window_size, size_t segment_size) {
  SetupTopology(window_size, segment_size);

  std::cout << "Starting Simulation (W=" << window_size
            << ", L=" << segment_size << ")..." << std::endl;

  // Start Traffic
  nodeA_->GetApp()->StartTransmission();

  // Run Engine
  engine_.Run();

  // Collect Results
  size_t total_bytes = nodeB_->GetApp()->GetTotalBytesReceived();
  SimTime duration = engine_.Now();

  double duration_sec = duration.count() / 1'000'000.0;
  double goodput = 0.0;
  if (duration_sec > 0) {
    goodput = (total_bytes * 8.0) / duration_sec;
  }

  return {goodput, duration, total_bytes};
}

void Simulator::SetupTopology(uint32_t window_size, size_t segment_size) {
  // Config Node A (Sender)
  NodeConfig configA;
  configA.window_size = window_size;
  configA.segment_size = segment_size;
  configA.is_sender = true;
  configA.prop_delay = PROPAGATION_DELAY_FWD();

  nodeA_ = std::make_shared<Node>(engine_, configA);

  // Config Node B (Receiver)
  NodeConfig configB;
  configB.window_size = window_size;
  configB.segment_size = segment_size;
  configB.is_sender = false;
  configB.prop_delay = PROPAGATION_DELAY_RECV();

  nodeB_ = std::make_shared<Node>(engine_, configB);

  // Create Channel with forward and reverse propagation delays
  channel_ = std::make_shared<Channel>(PROPAGATION_DELAY_FWD(), PROPAGATION_DELAY_REV(), engine_);

  // Connect the two PhysicalLayers through the Channel
  channel_->Connect(nodeA_->GetPhy(), nodeB_->GetPhy());
}

} // namespace ARQ
