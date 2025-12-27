#include "TransportLayer.hpp"
#include "ApplicationLayer.hpp"
#include "LinkLayer.hpp"
#include "Memory.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
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

void TransportLayer::SendDown(std::vector<std::byte> app_data_chunk) {
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

void TransportLayer::ReceiveUp(TransportHeader header,
                               std::pmr::vector<std::byte> payload) {
  (void)header;
  if (auto app = app_layer_.lock()) {
    app->Receive(std::span<const std::byte>(payload.data(), payload.size()));
  }
}

void TransportLayer::OnAppBufferAvailable() { link_layer_->SetPaused(false); }

} // namespace ARQ
