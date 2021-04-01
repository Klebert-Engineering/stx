#include <catch2/catch.hpp>

#include "stx/format.h"

#include <cstring>

using namespace std::string_literals;

SCENARIO("format a string", "[stx::format::format]") {
    GIVEN("A format string without any placeholder") {
        auto fmt = GENERATE("", "test", "test string", ":-)");
        auto r = stx::format(fmt);

        THEN("Expecting the result to be equal to the format string") {
            REQUIRE(r == fmt);
        }
    }

    GIVEN("A format string without any placeholder, with escaped braces") {
        std::pair<std::string_view, std::string_view> fmts[] = {
            {"{",             ""},
            {"{{",            "{"},
            {"{{{{",          "{{"},
            {"1{{2{{3",       "1{2{3"},
            {"}",             ""},
            {"}}",            "}"},
            {"}}}}",          "}}"},
            {"1{{2}}3",       "1{2}3"},
            {"123{{abc}}321", "123{abc}321"},
        };

        THEN("Expecting the result to be equal to the format string") {
            for (const auto& [fmt, expected] : fmts) {
                INFO("Format: " << fmt);
                INFO("Expected: " << expected);

                auto r = stx::format(fmt);

                REQUIRE(r == expected);
            }
        }
    }

    GIVEN("A format string with placeholders but no values") {
        auto fmt = GENERATE("{}", "{}{}", "{1}{0}", "{:}{9:abc}");

        THEN("Expecting the result to be empty") {
            auto r = stx::format(fmt);

            REQUIRE(r.empty());
        }
    }

    GIVEN("A format string with no indices") {
        auto fmt = "{}, {}, {}, {}, {}";

        THEN("Expecting the right order of replacements") {
            auto r = stx::format(fmt, 1, 2, 3, 4, 5);

            REQUIRE(r == "1, 2, 3, 4, 5");
        }
    }

    GIVEN("A format string with manual indices") {
        auto fmt = "{4}, {3}, {2}, {1}, {0}";

        THEN("Expecting the right order of replacements") {
            auto r = stx::format(fmt, 1, 2, 3, 4, 5);

            REQUIRE(r == "5, 4, 3, 2, 1");
        }
    }

    GIVEN("A format string with mixed type of indices") {
        auto fmt = "{4}, {}, {2}, {}, {3}";

        THEN("Expecting the right order of replacements") {
            auto r = stx::format(fmt, 1, 2, 3, 4, 5);

            REQUIRE(r == "5, 1, 3, 2, 4");
        }
    }

    GIVEN("A format string with an index out of range") {
        auto fmt = "a={99}, b={999}";

        THEN("Expecting no replacements") {
            auto r = stx::format(fmt, 1, 2, 3);

            REQUIRE(r == "a=, b=");
        }
    }

    GIVEN("A format string with justify left") {
        auto fmt = "{1:10}{:.<10}{:.<10}";

        THEN("Expecting a left aligned string with . as fill char") {
            auto r = stx::format(fmt, 123, "test");

            REQUIRE(r == "test      123.......test......");
        }
    }

    GIVEN("A format string with justify right") {
        auto fmt = "{0:10}{:.>10}{:.>10}";

        THEN("Expecting a right aligned string with . as fill char") {
            auto r = stx::format(fmt, 123, "test");

            REQUIRE(r == "       123.......123......test");
        }
    }

    GIVEN("A format string with justify center") {
        auto fmt = "{:.^10}{:-^10}{:,^7}{:=^7}";

        THEN("Expecting a right aligned string with . as fill char") {
            auto r = stx::format(fmt, 123, "test", 123, "test");

            REQUIRE(r == "...123....---test---,,123,,=test==");
        }
    }

    GIVEN("A format string with different integer bases") {
        auto fmt = "{0:0o},{0:0d},{0:0x}";

        THEN("Expecting a string containing the same number formatted with different bases") {
            auto r = stx::format(fmt, 123);

            REQUIRE(r == "173,123,7b");
        }

        THEN("Expecting a string containing the same number formatted with different bases (negative input)") {
            auto r = stx::format(fmt, -123);

            REQUIRE(r == "-173,-123,-7b");
        }
    }

    GIVEN("A format string with a fillc and no width specified") {
        auto fmt = "Hello{: }.";

        THEN("Expecting a string with a space inbetween, if the template replaces with an non-empty string") {
            auto r1 = stx::format(fmt, "Johannes");
            REQUIRE(r1 == "Hello Johannes.");

            auto r2 = stx::format(fmt, "");
            REQUIRE(r2 == "Hello.");
        }
    }

    GIVEN("A format string with mixed types") {
        auto fmt = "Ich {} {}l Wein mit {} Sorten {}.";

        THEN("Expecting correctly formatted arguments.") {
            auto r1 = stx::format(fmt, "mag", 1.5, 4.f, "Käse");
            REQUIRE(r1 == "Ich mag 1.5l Wein mit 4 Sorten Käse.");
        }
    }
}
