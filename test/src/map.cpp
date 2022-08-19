#include <catch2/catch_test_macros.hpp>

#include <map>
#include <unordered_map>
#include <type_traits>

#include "stx/map.h"

/* value_or is allowed to return a const&, if the map and the fallback are
 * non-rvalue references. Otherwise a reference to an out-of-scope object could
 * be returned. */
SCENARIO("value_or does not copy lvalue references", "[stx::map::value_or]") {
    GIVEN("A map") {
        std::map<int, int> m;

        WHEN("Called with a rvalue reference as fallback value") {
            THEN("Expecting a non-lvalue return type") {
                REQUIRE(!std::is_lvalue_reference_v<decltype(stx::value_or(m, -1, int{}))>);
            }
        }

        WHEN("Called with a lvalue reference as fallback value") {
            THEN("Expecting a lvalue return type") {
                int fallback;

                REQUIRE(std::is_lvalue_reference_v<decltype(stx::value_or(m, -1, fallback))>);
                REQUIRE(std::is_same_v<const int&, decltype(stx::value_or(m, -1, fallback))>);
            }
        }
    }

    GIVEN("A rvalue map") {
        WHEN("Called with a rvalue reference as fallback value") {
            THEN("Expecting a non-lvalue return type") {
                REQUIRE(!std::is_lvalue_reference_v<decltype(stx::value_or(std::map<int, const int>{}, -1, int{}))>);
            }
        }

        WHEN("Called with a lvalue reference as fallback value") {
            THEN("Expecting a non-lvalue return type") {
                int fallback;

                REQUIRE(!std::is_lvalue_reference_v<decltype(stx::value_or(std::map<int, const int>{}, -1, fallback))>);
            }
        }
    }
}

SCENARIO("value_or returns the fallback value if no match could be found", "[stx::map::value_or]") {
    GIVEN("An empty map") {
        std::map<int, int> m;

        WHEN("Called with a key") {
            auto r = stx::value_or(m, -1, 1);

            THEN("Expecting the fallback value being returned") {
                REQUIRE(r == 1);
            }
        }
    }

    GIVEN("An non-empty map") {
        std::map<int, int> m{{0, 0}};

        WHEN("Called with a key that does not exist") {
            auto r = stx::value_or(m, -1, 1);

            THEN("Expecting the fallback value being returned") {
                REQUIRE(r == 1);
            }
        }
    }
}

SCENARIO("value_or returns the matching value if the key exists", "[stx::map::value_or]") {
    GIVEN("A map") {
        std::map<int, int> m{{0, 1}};

        WHEN("Called with an existing key") {
            auto r = stx::value_or(m, 0, -1);

            THEN("Expecting the value for the key being returned") {
                REQUIRE(r == 1);
            }
        }
    }
}
