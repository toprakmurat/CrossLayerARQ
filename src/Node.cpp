#include "Node.hpp"

namespace ARQ {

Node::Node(SimulatorEngine &engine, const NodeConfig &config) {
  // 1. Instantiate (Strong Ownership)
  phy_ = std::make_shared<PhysicalLayer>(engine, config.bit_rate,
                                         config.prop_delay);
  link_ = std::make_shared<LinkLayer>(engine, config.window_size);
  trans_ = std::make_shared<TransportLayer>(config.segment_size);
  app_ = std::make_shared<ApplicationLayer>(config.is_sender);

  // 2. Wire Internals (Top-Down / Bot-Up)
  // Using simple SetUpper/Lower which now store weak_ptrs

  // Phy <-> Link
  phy_->SetUpperLayer(link_);
  link_->SetLowerLayer(phy_);

  // Link <-> Trans
  link_->SetUpperLayer(trans_);
  trans_->SetLowerLayer(link_);

  // Trans <-> App
  trans_->SetUpperLayer(app_);
  app_->SetLowerLayer(trans_);
}

void Node::ConnectPeer(std::shared_ptr<Node> peer) {
  if (peer && peer->GetPhy()) {
    phy_->SetPeer(peer->GetPhy());
  }
}

} // namespace ARQ
