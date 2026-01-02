#pragma once

#include <chrono>
#include <cstdint>
#include <source_location>

namespace ARQ {

consteval uint64_t BIT_RATE_BPS() { return 10'000'000; } // 10 Mbps

// Delays in milliseconds, refer to homework document
consteval std::chrono::milliseconds PROPAGATION_DELAY_FWD() {
  return std::chrono::milliseconds(40);
}
consteval std::chrono::milliseconds PROPAGATION_DELAY_RECV() {
  return std::chrono::milliseconds(10);
}
consteval std::chrono::milliseconds PROCESSING_DELAY() {
  return std::chrono::milliseconds(2);
}

// Error Model Constants
consteval double BER_GOOD() { return 1e-6; }
consteval double BER_BAD() { return 5e-3; }
consteval double P_GOOD_TO_BAD() { return 0.002; }
consteval double P_BAD_TO_GOOD() { return 0.05; }

// --- Application Constants ---
consteval size_t TOTAL_FILE_SIZE_BYTES() { return 100 * 1024 * 1024; } // 100 MB
consteval size_t RECEIVER_BUFFER_SIZE_BYTES() { return 256 * 1024; }   // 256 KB

// --- Logging ---
inline void
Log(std::string_view msg,
    const std::source_location &loc = std::source_location::current()) {
  (void)msg;
  (void)loc;
}

} // namespace ARQ
