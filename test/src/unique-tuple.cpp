#include <catch2/catch_all.hpp>

#include "stx/unique-tuple.h"

SCENARIO("unique tuple", "[stx::unique_tuple]") {
    GIVEN("A tuple with unique types") {
        auto t = stx::unique_tuple<int, float, bool>();

        THEN("Expecting a tuple size of three") {
            REQUIRE(std::tuple_size_v<decltype(t)> == 3);
        }
    }

    GIVEN("A tuple with repeating types") {
        auto t = stx::unique_tuple<bool, int, float, int, bool, float, float>();

        THEN("Expecting a tuple size of three") {
            REQUIRE(std::tuple_size_v<decltype(t)> == 3);
        }
    }
}
