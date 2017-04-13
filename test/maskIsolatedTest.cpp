#include "../source/maskIsolated.hpp"

#include "catch.hpp"

struct Event {
    uint64_t x;
    uint64_t y;
    uint_fast64_t timestamp;
};

TEST_CASE("Filter out events with low spatial or in time activity", "[MaskIsolated]") {
    auto maskIsolated = tarsier::make_maskIsolated<Event, 304, 240, 10>([](Event event) -> void {
        REQUIRE(event.x == 100);
    });
    maskIsolated(Event{200, 200, 0});
    maskIsolated(Event{200, 202, 1});
    maskIsolated(Event{200, 201, 20});
    maskIsolated(Event{100, 100, 40});
    maskIsolated(Event{100, 101, 41});
}
