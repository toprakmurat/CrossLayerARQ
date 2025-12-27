#include "ApplicationLayer.hpp"
#include "TransportLayer.hpp"
#include <iostream>

namespace ARQ {

ApplicationLayer::ApplicationLayer(bool is_sender) : is_sender_(is_sender) {}

void ApplicationLayer::SetLowerLayer(
    std::shared_ptr<TransportLayer> transport) {
  transport_ = transport;
}

void ApplicationLayer::StartTransmission() {
  if (!is_sender_)
    return;

  // Generate one chunk
  GenerateChunk();
}

void ApplicationLayer::GenerateChunk() {
  // 4KB Chunk
  size_t chunk_size = 4096;
  if (total_bytes_sent_ >= TOTAL_FILE_SIZE_BYTES())
    return; // Cap at 100MB

  std::vector<std::byte> chunk(chunk_size);
  // Fill dummy

  transport_->SendDown(std::move(chunk));
  total_bytes_sent_ += chunk_size;

  // Continue?
  // In a real loop we'd schedule next. Here we just do one for now to match
  // main loop test.
}

void ApplicationLayer::Receive(std::span<const std::byte> data_view) {
  total_bytes_received_ += data_view.size();
  current_buffer_usage_ += data_view.size();

  if (current_buffer_usage_ >= RECEIVER_BUFFER_SIZE_BYTES()) {
    // Backpressure handling is implicit via
    // TransportLayer::OnAppBufferAvailable and SetPaused logic.
  }
}

void ApplicationLayer::ConsumeData(size_t bytes_processed) {
  if (bytes_processed > current_buffer_usage_) {
    current_buffer_usage_ = 0;
  } else {
    current_buffer_usage_ -= bytes_processed;
  }

  if (current_buffer_usage_ < RECEIVER_BUFFER_SIZE_BYTES() / 2) {
    if (auto t = transport_)
      t->OnAppBufferAvailable();
  }
}

} // namespace ARQ
