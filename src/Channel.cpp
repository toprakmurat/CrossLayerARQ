#include "Channel.hpp"
#include <random>
#include <set>
#include <iostream>

namespace ARQ
{

  Channel::Channel(std::chrono::milliseconds forward_delay, std::chrono::milliseconds reverse_delay, SimulatorEngine &engine)
      : state_(State::GOOD),
        engine_(engine),
        forward_propagation_delay_(forward_delay),
        reverse_propagation_delay_(reverse_delay) {}

  void Channel::Connect(std::shared_ptr<PhysicalLayer> phyA, std::shared_ptr<PhysicalLayer> phyB)
  {
    phyA_ = phyA;
    phyB_ = phyB;
    phyA_->SetPeer(phyB_);
    phyB_->SetPeer(phyA_);

    // Set this channel on both physical layers
    phyA_->SetChannel(shared_from_this());
    phyB_->SetChannel(shared_from_this());
  }

  std::chrono::milliseconds Channel::GetPropagationDelay(std::shared_ptr<PhysicalLayer> sender) const
  {
    if (sender == phyA_)
    {
      return forward_propagation_delay_;
    }
    else if (sender == phyB_)
    {
      return reverse_propagation_delay_;
    }
    else
    {
      throw std::invalid_argument("Sender not connected to this channel");
    }
  }

  void Channel::UpdateChannelState()
  {
    double roll = dist_(rng_);
    if (state_ == State::GOOD)
    {
      if (roll < P_GOOD_TO_BAD())
      {
        state_ = State::BAD;
        std::cout << "[Channel] State changed to BAD." << std::endl;
      }
    }
    else
    {
      if (roll < P_BAD_TO_GOOD())
      {
        state_ = State::GOOD;
        std::cout << "[Channel] State changed to GOOD." << std::endl;
      }
    }
  }

  void Channel::Transmit(std::shared_ptr<PhysicalLayer> sender, std::shared_ptr<const Frame> frame, SimTime delay)
  {
    double ber = (state_ == State::GOOD) ? BER_GOOD() : BER_BAD();
    size_t total_bytes = sizeof(FrameHeader) + frame->payload.size();
    size_t total_bits = total_bytes * 8;
    double per = 1.0 - std::pow(1.0 - ber, static_cast<double>(total_bits));
    bool has_error = (dist_(rng_) < per);


    std::shared_ptr<PhysicalLayer> receiver = (sender == phyA_) ? phyB_ : phyA_;

    engine_.Schedule(delay, [this, receiver, frame, has_error]()
                     { receiver->OnFrameArrival(frame, has_error); });

    // Update channel state after each transmission
    UpdateChannelState();
  }

}
