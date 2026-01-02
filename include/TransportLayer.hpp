#pragma once

#include "Packets.hpp"
#include <memory>
#include <span>

namespace ARQ {

class LinkLayer;
class ApplicationLayer;

class TransportLayer : public std::enable_shared_from_this<TransportLayer> {
public:
  TransportLayer(size_t segment_size);

  void SetLowerLayer(std::shared_ptr<LinkLayer> link);
  void SetUpperLayer(std::shared_ptr<ApplicationLayer> app);

  // --- SENDER API (Downstream) ---
  void SendDown(std::span<const std::byte> app_data_chunk);

  // --- RECEIVER API (Upstream) ---
  bool ReceiveUp(TransportHeader header,
                 const std::pmr::vector<std::byte> &payload);

  // --- CONTROL API ---
  void OnAppBufferAvailable();

private:
  std::shared_ptr<LinkLayer> link_layer_;
  std::weak_ptr<ApplicationLayer> app_layer_;
  size_t segment_size_;
  size_t current_buffer_usage_ = 0;
};

} // namespace ARQ
