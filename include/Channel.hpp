#pragma once

#include <chrono>
#include <memory>
#include "Common.hpp"
#include "PhysicalLayer.hpp"

namespace ARQ
{
  class Channel : public std::enable_shared_from_this<Channel>
  {
  private:
    enum class State
    {
      GOOD,
      BAD
    };
    State state_;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_{0.0, 1.0};
    SimulatorEngine &engine_;

    std::chrono::milliseconds forward_propagation_delay_;
    std::chrono::milliseconds reverse_propagation_delay_;
    std::shared_ptr<PhysicalLayer> phyA_;
    std::shared_ptr<PhysicalLayer> phyB_;

  public:
    Channel(std::chrono::milliseconds forward_delay, std::chrono::milliseconds reverse_delay, SimulatorEngine &engine);

    void Connect(std::shared_ptr<PhysicalLayer> phyA, std::shared_ptr<PhysicalLayer> phyB);

    std::chrono::milliseconds GetPropagationDelay(std::shared_ptr<PhysicalLayer> sender) const;

    void UpdateChannelState();

    void Transmit(std::shared_ptr<PhysicalLayer> sender, std::shared_ptr<const Frame> frame, SimTime delay);
  };

}