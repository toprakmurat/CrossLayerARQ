#include "Engine.hpp"

namespace ARQ {

uint64_t SimulatorEngine::Schedule(SimTime delay,
                                   std::function<void()> callback) {
  uint64_t id = next_event_id_++;
  SimTime execution_time = current_time_ + delay;
  events_.push({execution_time, id, std::move(callback)});
  return id;
}

void SimulatorEngine::Cancel(uint64_t event_id) {
  cancelled_events_.insert(event_id);
}

void SimulatorEngine::Run() {
  while (!events_.empty()) {
    Event evt = events_.top();
    events_.pop();

    // Lazy Cancellation check
    if (cancelled_events_.contains(evt.id)) {
      cancelled_events_.erase(evt.id);
      continue;
    }

    // Advance time
    if (evt.time > current_time_) {
      current_time_ = evt.time;
    }

    // Execute
    if (evt.callback) {
      evt.callback();
    }
  }
}

} // namespace ARQ
