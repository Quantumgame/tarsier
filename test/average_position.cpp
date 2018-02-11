#include "../source/average_position.hpp"
#include "../third_party/Catch2/single_include/catch.hpp"

struct event {
    uint16_t x;
    uint16_t y;
} __attribute__((packed));

struct position {
    double x;
    double y;
} __attribute__((packed));

TEST_CASE("Average the position of the given events", "[average_position]") {
    auto first_received = false;
    auto average_position = tarsier::make_average_position<event, position>(
        0.0,
        0.0,
        0.5,
        [](event event, double x, double y) -> position {
            return {x, y};
        },
        [&](position position) -> void {
            if (first_received) {
                REQUIRE(position.x == 100);
                REQUIRE(position.y == 50);
            } else {
                first_received = true;
            }
        });
    average_position(event{0, 0});
    average_position(event{200, 100});
}
