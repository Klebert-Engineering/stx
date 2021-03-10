#include <catch2/catch.hpp>

#include "stx/string.h"

SCENARIO("split a string into parts", "[stx::string::split]") {
    GIVEN("An empty string") {
        WHEN("Called with '/' as separator (discard empty)") {
            auto r = stx::split("", "/", true);

            THEN("Expecting empty result") {
                REQUIRE(r.empty());
            }
        }
        WHEN("Called with '/' as separator (preserve empty)") {
            auto r = stx::split("", "/", false);

            THEN("Expecting empty result") {
                REQUIRE(r.empty());
            }
        }
        WHEN("Called with an empty string as separator (preserve empty)") {
            auto r = stx::split("", "", false);

            THEN("Expecting empty result") {
                REQUIRE(r.empty());
            }
        }
    }
    GIVEN("A short string") {
        WHEN("Called with an empty separator") {
            auto r = stx::split("test", "");

            THEN("Expecting one element containing the whole string") {
                REQUIRE(r.size() == 1);
                REQUIRE(r.at(0) == "test");
            }
        }
    }
    GIVEN("A short, two element string") {
        WHEN("Called") {
            auto r = stx::split("a/b", "/");

            THEN("Expecting two elements (a, b)") {
                REQUIRE(r.size() == 2);
                REQUIRE(r.at(0) == "a");
                REQUIRE(r.at(1) == "b");
            }
        }
    }
    GIVEN("Strings with empty element(s) at the front, the back and in the middle") {
        WHEN("Called (front, remove empty)") {
            auto r1 = stx::split("/a", "/");
            auto r2 = stx::split("//a", "/");

            THEN("Expecting one element (a)") {
                REQUIRE(r1.size() == 1);
                REQUIRE(r1.at(0) == "a");

                REQUIRE(r2.size() == 1);
                REQUIRE(r2.at(0) == "a");
            }
        }
        WHEN("Called (back, remove empty)") {
            auto r1 = stx::split("a/", "/");
            auto r2 = stx::split("a//", "/");

            THEN("Expecting one element (a)") {
                REQUIRE(r1.size() == 1);
                REQUIRE(r1.at(0) == "a");

                REQUIRE(r2.size() == 1);
                REQUIRE(r2.at(0) == "a");
            }
        }
        WHEN("Called (front+back, remove empty)") {
            auto r1 = stx::split("/a/", "/");
            auto r2 = stx::split("//a//", "/");

            THEN("Expecting one element (a)") {
                REQUIRE(r1.size() == 1);
                REQUIRE(r1.at(0) == "a");

                REQUIRE(r2.size() == 1);
                REQUIRE(r2.at(0) == "a");
            }
        }
        WHEN("Called (middle, remove empty)") {
            auto r1 = stx::split("a//b", "/");
            auto r2 = stx::split("a///b", "/");

            THEN("Expecting two elements (a, b)") {
                REQUIRE(r1.size() == 2);
                REQUIRE(r1.at(0) == "a");
                REQUIRE(r1.at(1) == "b");

                REQUIRE(r2.size() == 2);
                REQUIRE(r2.at(0) == "a");
                REQUIRE(r2.at(1) == "b");
            }
        }
    }
    GIVEN("String with multiple elements") {
        auto r = stx::split("a/b/c/d/e", "/");

        THEN("Expecting five elements (a, b, c, d, e)") {
            REQUIRE(r.size() == 5);
            REQUIRE(r.at(0) == "a");
            REQUIRE(r.at(1) == "b");
            REQUIRE(r.at(2) == "c");
            REQUIRE(r.at(3) == "d");
            REQUIRE(r.at(4) == "e");
        }
    }
}

SCENARIO("join a range of strings together", "[stx::string::join]") {
    GIVEN("An empty range") {
        WHEN("Called with an empty separator") {
            auto range = std::vector<std::string> {};
            auto r = stx::join(range.begin(), range.end(), "");

            THEN("Expecting an empty string") {
                REQUIRE(r == "");
            }
        }
        WHEN("Called with a non-empty separator") {
            auto range = std::vector<std::string> {};
            auto r = stx::join(range.begin(), range.end(), "/");

            THEN("Expecting an empty string") {
                REQUIRE(r == "");
            }
        }
    }
    GIVEN("A single item") {
        WHEN("Called with an empty separator") {
            auto range = std::vector<std::string> {"A"};
            auto r = stx::join(range.begin(), range.end(), "");

            THEN("A string containing the single item") {
                REQUIRE(r == "A");
            }
        }
        WHEN("Called with a non-empty separator") {
            auto range = std::vector<std::string> {"A"};
            auto r = stx::join(range.begin(), range.end(), "/");

            THEN("A string containing the single item") {
                REQUIRE(r == "A");
            }
        }
    }
    GIVEN("Multiple items") {
        WHEN("Called with an empty separator") {
            auto range = std::vector<std::string> {"A", "B", "C"};
            auto r = stx::join(range.begin(), range.end(), "");

            THEN("A string containing all items") {
                REQUIRE(r == "ABC");
            }
        }
        WHEN("Called with a non-empty separator") {
            auto range = std::vector<std::string> {"A", "B", "C"};
            auto r = stx::join(range.begin(), range.end(), "/");

            THEN("A string containing all items separated by /") {
                REQUIRE(r == "A/B/C");
            }
        }
        WHEN("Called with a multi-character separator") {
            auto range = std::vector<std::string> {"A", "B", "C"};
            auto r = stx::join(range.begin(), range.end(), "<:-)>");

            THEN("A string containing all items separated by <:-)>") {
                REQUIRE(r == "A<:-)>B<:-)>C");
            }
        }
    }
}

TEST_CASE("String List Replace", "[stx::string::replace_with]") {
    SECTION("Empty Template") {
        auto res = stx::replace_with("", "?", 1, 2, 3);

        REQUIRE(res == "");
    }

    SECTION("Single Replacement") {
        auto res = stx::replace_with("?", "?", "test");

        REQUIRE(res == "test");
    }

    SECTION("Replacement with text") {
        auto res = stx::replace_with("head ? tail", "?", "test");

        REQUIRE(res == "head test tail");
    }

    SECTION("Too few values") {
        auto res = stx::replace_with("?,?,?", "?", "test");

        REQUIRE(res == "test,,");
    }

    SECTION("Multiple Replacements") {
        auto res = stx::replace_with("?,?,?", "?", "test", 123, true);

        REQUIRE(res == "test,123,1");
    }
}
