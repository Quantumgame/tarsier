#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace tarsier {

/// ComputeActivity evaluates the activity within a temporal neighbourhood
template <typename Event, typename ActivityEvent, uint64_t lifespan,
          typename ActivityEventFromEvent, typename HandleActivityEvent>
class ComputeActivity {
public:
  ComputeActivity(ActivityEventFromEvent activityEventFromEvent,
                  HandleActivityEvent handleActivityEvent)
      : _activityEventFromEvent(
            std::forward<ActivityEventFromEvent>(activityEventFromEvent)),
        _handleActivityEvent(
            std::forward<HandleActivityEvent>(handleActivityEvent)),
        _lifespan(lifespan), _activity(0), _lastTimeStamp(0) {}

  ComputeActivity(const ComputeActivity &) = delete;
  ComputeActivity(ComputeActivity &&) = default;
  ComputeActivity &operator=(const ComputeActivity &) = delete;
  ComputeActivity &operator=(ComputeActivity &&) = default;
  virtual ~ComputeActivity() {}

  virtual void operator()(Event event) {
    _activity *=
        exp(-static_cast<double>(event.timestamp - _lastTimeStamp) / _lifespan);
    _activity += 1;
    _lastTimeStamp = event.timestamp;
    _handleActivityEvent(_activityEventFromEvent(event, _activity));
  }

protected:
  ActivityEventFromEvent _activityEventFromEvent;
  const uint64_t _lifespan;
  double _activity;
  uint64_t _lastTimeStamp;
  HandleActivityEvent _handleActivityEvent;
};

template <typename Event, typename ActivityEvent, uint64_t lifespan,
          typename ActivityEventFromEvent, typename HandleActivityEvent>
ComputeActivity<Event, ActivityEvent, lifespan, ActivityEventFromEvent,
                HandleActivityEvent>
make_computeActivity(ActivityEventFromEvent activityEventFromEvent,
                     HandleActivityEvent handleActivityEvent) {
  return ComputeActivity<Event, ActivityEvent, lifespan, ActivityEventFromEvent,
                         HandleActivityEvent>(
      std::forward<ActivityEventFromEvent>(activityEventFromEvent),
      std::forward<HandleActivityEvent>(handleActivityEvent));
}
}
