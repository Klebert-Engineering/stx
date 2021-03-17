#include <catch2/catch.hpp>

#include "stx/string.h"

#include <cstring>

using namespace std::string_literals;

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
    GIVEN("String with multiple elements - return string_view") {
        std::string_view in = "error/a/b/c/d/e/error";

        /* Check that we do not read over the string_views boundaries! */
        in.remove_prefix(6);
        in.remove_suffix(6);

        auto r = stx::split<std::vector<std::string_view>>(in, "/");

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

TEST_CASE("Range to hex string", "[stx::string::to_hex]") {
    SECTION("Empty Range") {
        auto vec = std::vector<unsigned>();
        auto res = stx::to_hex(vec.begin(), vec.end());

        REQUIRE(res == "");
    }

    SECTION("uint8 vector") {
        auto vec = std::vector<std::uint8_t> {
            0x0, 0x1, 0xff, 0xf0
        };
        auto resl = stx::to_hex(vec.begin(), vec.end());
        auto resu = stx::to_hex(vec.begin(), vec.end(), true);

        REQUIRE(resl == "0001fff0");
        REQUIRE(resu == "0001FFF0");
    }

    SECTION("uint32 vector") {
        auto vec = std::vector<std::uint32_t> {
            0x0, 0x1, 0xff, 0xff00ff00
        };
        auto resl = stx::to_hex(vec.begin(), vec.end());
        auto resu = stx::to_hex(vec.begin(), vec.end(), true);

        REQUIRE(resl == "0000000000000001000000ffff00ff00");
        REQUIRE(resu == "0000000000000001000000FFFF00FF00");
    }

    SECTION("string") {
        auto str = "Hallo Welt!"s;

        auto resl = stx::to_hex(str.begin(), str.end());
        auto resu = stx::to_hex(str.begin(), str.end(), true);

        REQUIRE(resl == "48616c6c6f2057656c7421");
        REQUIRE(resu == "48616C6C6F2057656C7421");
    }

    SECTION("c-string") {
        auto str = "Hallo Welt!";

        auto resl = stx::to_hex(str, str + std::strlen(str));
        auto resu = stx::to_hex(str, str + std::strlen(str), true);

        REQUIRE(resl == "48616c6c6f2057656c7421");
        REQUIRE(resu == "48616C6C6F2057656C7421");
    }
}
