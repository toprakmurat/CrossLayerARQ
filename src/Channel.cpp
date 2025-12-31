#include "Channel.hpp"
#include <random>
#include <set>

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
      }
    }
    else
    {
      if (roll < P_BAD_TO_GOOD())
      {
        state_ = State::GOOD;
      }
    }
  }

  void Channel::Transmit(std::shared_ptr<PhysicalLayer> sender, std::shared_ptr<const Frame> frame, SimTime delay)
  {
    double ber = (state_ == State::GOOD) ? BER_GOOD() : BER_BAD();
    size_t total_bytes = sizeof(FrameHeader) + frame->payload.size();
    size_t total_bits = total_bytes * 8;

    // Compute number of bit errors based on BER
    std::binomial_distribution<size_t> error_dist(total_bits, ber);
    size_t num_errors = error_dist(rng_);

    // Create a modifiable copy of the frame
    auto corrupted_frame = std::make_shared<Frame>(*frame);

    // Choose random bit positions and flip them
    std::uniform_int_distribution<size_t> bit_pos_dist(0, total_bits - 1);
    std::set<size_t> error_positions;
    while (error_positions.size() < num_errors)
    {
      error_positions.insert(bit_pos_dist(rng_));
    }

    // Flip bits at selected positions
    for (size_t bit_pos : error_positions)
    {
      size_t byte_index = bit_pos / 8;
      size_t bit_index = bit_pos % 8;

      if (byte_index < sizeof(FrameHeader))
      {
        // Flip bit in header
        uint8_t *header_bytes = reinterpret_cast<uint8_t *>(&corrupted_frame->header);
        header_bytes[byte_index] ^= (1 << bit_index);
      }
      else
      {
        // Flip bit in payload
        size_t payload_index = byte_index - sizeof(FrameHeader);
        corrupted_frame->payload[payload_index] ^= std::byte(1 << bit_index);
      }
    }

    std::shared_ptr<PhysicalLayer> receiver = (sender == phyA_) ? phyB_ : phyA_;

    engine_.Schedule(delay, [this, receiver, corrupted_frame]()
                     { receiver->OnFrameArrival(corrupted_frame); });
  }

}
