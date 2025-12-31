#include "CRC32.hpp"

CRC32::CRC32()
{
  // Precompute the CRC table
  for (uint32_t i = 0; i < 256; i++)
  {
    uint32_t remainder = i;
    for (uint32_t j = 0; j < 8; j++)
    {
      if (remainder & 1)
      {
        remainder = (remainder >> 1) ^ polynomial;
      }
      else
      {
        remainder >>= 1;
      }
    }
    table[i] = remainder;
  }
}

// Function to calculate CRC for a given buffer
uint32_t CRC32::calculate(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xFFFFFFFF; // Initial value
  for (size_t i = 0; i < length; i++)
  {
    uint8_t byte = data[i];
    uint8_t lookupIndex = (crc ^ byte) & 0xFF;
    crc = (crc >> 8) ^ table[lookupIndex];
  }
  return crc ^ 0xFFFFFFFF; // Final XOR
}
