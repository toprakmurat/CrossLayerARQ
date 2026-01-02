#include "ApplicationLayer.hpp"
#include "Common.hpp"
#include "TransportLayer.hpp"

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
  size_t chunk_size = 4096;

  while (total_bytes_sent_ < TOTAL_FILE_SIZE_BYTES()) {
    size_t remaining = TOTAL_FILE_SIZE_BYTES() - total_bytes_sent_;
    size_t s = std::min(chunk_size, remaining);

    std::vector<std::byte> chunk(s);
    // TODO: Fill dummy data, currently it's empty

    transport_->SendDown(chunk); // Implicit conversion to span<const byte>
    total_bytes_sent_ += s;
  }
}

bool ApplicationLayer::Receive(std::span<const std::byte> data_view) {
  if (current_buffer_usage_ + data_view.size() > RECEIVER_BUFFER_SIZE_BYTES()) {
    return false;
  }

  total_bytes_received_ += data_view.size();
  current_buffer_usage_ += data_view.size();
  return true;
}

void ApplicationLayer::ConsumeData(size_t bytes_processed) {
  if (bytes_processed > current_buffer_usage_) {
    current_buffer_usage_ = 0;
  } else {
    current_buffer_usage_ -= bytes_processed;
  }

  // Signal when there is available space
  if (current_buffer_usage_ < RECEIVER_BUFFER_SIZE_BYTES()) {
    if (auto t = transport_) {
      t->OnAppBufferAvailable();
    }
  }
}

} // namespace ARQ
