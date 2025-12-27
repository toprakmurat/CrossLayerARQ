#pragma once

#include "Common.hpp"
#include "Packets.hpp"
#include <memory>
#include <vector>

namespace ARQ {

class LinkLayer;
class ApplicationLayer;

class TransportLayer : public std::enable_shared_from_this<TransportLayer> {
public:
  TransportLayer(size_t segment_size);

  void SetLowerLayer(std::shared_ptr<LinkLayer> link);
  void SetUpperLayer(std::shared_ptr<ApplicationLayer> app);

  // --- SENDER API (Downstream) ---
  void SendDown(std::vector<std::byte> app_data_chunk);

  // --- RECEIVER API (Upstream) ---
  void ReceiveUp(TransportHeader header, std::pmr::vector<std::byte> payload);

  // --- CONTROL API ---
  void OnAppBufferAvailable();

private:
  std::shared_ptr<LinkLayer> link_layer_;
  std::weak_ptr<ApplicationLayer> app_layer_;
  size_t segment_size_;
};

} // namespace ARQ
