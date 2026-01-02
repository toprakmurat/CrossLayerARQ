#pragma once

#include <chrono>
#include <functional>
#include <queue>
#include <set>
#include <vector>

namespace ARQ {

using SimTime = std::chrono::microseconds;

struct Event {
  SimTime time;
  uint64_t id;
  std::function<void()> callback;

  // For priority queue: smallest time first
  bool operator>(const Event &other) const { return time > other.time; }

  // PMR-aware allocator considerations:
  // Standard std::function is used for callback storage.
};

class SimulatorEngine {
public:
  SimulatorEngine() = default;

  SimTime Now() const { return current_time_; }

  // Schedules an event. Returns ID for cancellation.
  uint64_t Schedule(SimTime delay, std::function<void()> callback);

  // Cancel an event by ID
  void Cancel(uint64_t event_id);

  // Run simulation until empty
  void Run();

private:
  SimTime current_time_{0};
  uint64_t next_event_id_{1};

  std::priority_queue<Event, std::vector<Event>, std::greater<Event>> events_;
  std::set<uint64_t> cancelled_events_;
};

} // namespace ARQ
