#pragma once

#include "Common.hpp"
#include <memory>
#include <span>
#include <vector>

namespace ARQ {

class TransportLayer;

class ApplicationLayer : public std::enable_shared_from_this<ApplicationLayer> {
public:
  ApplicationLayer(bool is_sender);

  void SetLowerLayer(std::shared_ptr<TransportLayer> transport);

  // --- SENDER API (Trigger) ---
  void StartTransmission();

  // --- RECEIVER API (Upstream) ---
  void Receive(std::span<const std::byte> data_view);

  // --- CONTROL API ---
  void ConsumeData(size_t bytes_processed);

  // Stats
  size_t GetTotalBytesReceived() const { return total_bytes_received_; }

private:
  std::shared_ptr<TransportLayer> transport_;
  bool is_sender_;

  size_t total_bytes_sent_ = 0;
  size_t total_bytes_received_ = 0;
  size_t current_buffer_usage_ = 0;

  void GenerateChunk();
};

} // namespace ARQ
