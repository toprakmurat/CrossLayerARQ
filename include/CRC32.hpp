#pragma once

#include <cstdint>
#include <cstddef>

class CRC32
{
private:
  uint32_t table[256];
  const uint32_t polynomial = 0xEDB88320; // Standard CRC-32 polynomial

public:
  CRC32();

  // Function to calculate CRC for a given buffer
  uint32_t calculate(const uint8_t *data, size_t length);
};