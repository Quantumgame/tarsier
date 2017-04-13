#include "../source/stitch.hpp"

#include "catch.hpp"

struct ThresholdCrossing {
    uint64_t x;
    uint64_t y;
    uint_fast64_t timestamp;
    bool isSecond;
};

struct Event {
    uint64_t x;
    uint64_t y;
    uint_fast64_t timeDelta;
};

TEST_CASE("Stitch an threshold crossings stream", "[Stitch]") {
    auto stitch = tarsier::make_stitch<ThresholdCrossing, Event, 304, 240>(
        [](const ThresholdCrossing& secondThresholdCrossing, uint_fast64_t timeDelta) -> Event {
            return Event{secondThresholdCrossing.x, secondThresholdCrossing.y, timeDelta};
        },
        [](Event event) -> void {
            REQUIRE(event.timeDelta == 200);
        }
    );
    stitch(ThresholdCrossing{200, 100,   0, false});
    stitch(ThresholdCrossing{200,   0, 100, false});
    stitch(ThresholdCrossing{200, 100, 200,  true});
}
