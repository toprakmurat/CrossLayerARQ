#include "TransportLayer.hpp"
#include "ApplicationLayer.hpp"
#include "Common.hpp"
#include "LinkLayer.hpp"
#include "Memory.hpp"
#include <algorithm>
#include <cstring>
#include <vector>

namespace ARQ {

TransportLayer::TransportLayer(size_t segment_size)
    : segment_size_(segment_size) {}

void TransportLayer::SetLowerLayer(std::shared_ptr<LinkLayer> link) {
  link_layer_ = link;
}

void TransportLayer::SetUpperLayer(std::shared_ptr<ApplicationLayer> app) {
  app_layer_ = app;
}

void TransportLayer::SendDown(std::span<const std::byte> app_data_chunk) {
  size_t offset = 0;
  size_t total = app_data_chunk.size();

  while (offset < total) {
    size_t len = std::min(segment_size_, total - offset);

    std::pmr::vector<std::byte> segment(len, Memory::packets());
    std::copy_n(app_data_chunk.begin() + offset, len, segment.begin());

    link_layer_->Send(std::move(segment));

    offset += len;
  }
}

bool TransportLayer::ReceiveUp(TransportHeader header,
                               const std::pmr::vector<std::byte> &payload) {
  (void)header;

  if (current_buffer_usage_ + payload.size() > RECEIVER_BUFFER_SIZE_BYTES()) {
    return false;
  }

  current_buffer_usage_ += payload.size();

  if (auto app = app_layer_.lock()) {
    bool accepted = app->Receive(
        std::span<const std::byte>(payload.data(), payload.size()));
    if (!accepted) {
      // Backpressure: Application buffer is full.
      // We return 'false' to the Link Layer to indicate delivery failed.
      // The Link Layer will stop passing up frames and hold them in its
      // control, eventually filling the Link window and stopping the physical
      // sender.

      // TODO: Do we need to do something extra?
      current_buffer_usage_ -= payload.size(); // Revert speculative usage add
      return false;
    }
  }
  return true;
}

void TransportLayer::OnAppBufferAvailable() { link_layer_->SetPaused(false); }

} // namespace ARQ
