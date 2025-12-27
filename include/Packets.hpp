#pragma once

#include <cstdint>
#include <memory_resource>
#include <vector>

namespace ARQ {

enum class FrameType : uint8_t {
  DATA,
  ACK,
  NACK, // Optional
  DUMMY
};

struct TransportHeader {
  uint32_t seq_num;
  uint32_t length;
} __attribute__((packed));
static_assert(sizeof(TransportHeader) == 8, "TransportHeader must be 8 bytes");

struct LinkHeader {
  FrameType type; // sizeof(FrameType) = 1
  uint8_t flags;
  uint16_t checksum;
  uint32_t seq_num;
  uint32_t ack_num;
  double timestamp;
  uint32_t padding[1]; // to reach 24 bytes total
} __attribute__((packed));
// 1(Type) + 1(Flags) + 2(CRC) + 4(Seq) + 4(Ack) + 8(Time) + 4(Pad) = 24 bytes
static_assert(sizeof(LinkHeader) == 24, "LinkHeader must be 24 bytes");

struct FrameHeader {
  LinkHeader link_header;
  TransportHeader transport_header;
} __attribute__((packed));

struct Frame {
  FrameHeader header;
  std::pmr::vector<std::byte> payload;

  Frame(std::pmr::memory_resource *mr) : payload{mr} {}
};

} // namespace ARQ
