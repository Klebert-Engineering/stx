#include <catch2/catch_all.hpp>

#include "stx/option-set.h"

#include <string>

struct A { STX_OPTION_OPERATORS() };
struct B { STX_OPTION_OPERATORS() };
struct C { STX_OPTION_OPERATORS() };
struct D { STX_OPTION_OPERATORS() };

namespace kw
{
STX_DECLARE_KWARG(NAME, std::string)
STX_DECLARE_KWARG(AGE,  int)
}

std::string my_function(stx::option_set<kw::NAME, kw::AGE> options)
{
    return options.get<kw::NAME>()->value + " (" + std::to_string(*options.get<kw::AGE>()) + ")";
}

SCENARIO("option set", "[stx::option_set]") {
    GIVEN("An empty option set") {
        (void)stx::option_set<>();
    }

    GIVEN("Merging multiple options into an option_set") {
        auto set = A() | B() | C();

        THEN("Expecting the option_set type to hold all option types") {
            REQUIRE(std::is_same_v<stx::option_set<A, B, C>, decltype(set)>);
        }

        THEN("Expecting all options to be set") {
            REQUIRE(set.has<A>());
            REQUIRE(set.has<B>());
            REQUIRE(set.has<C>());
        }

        THEN("Merging in duplicate types") {
            auto set2 = set | C() | B ();

            THEN("Expecting the new option_set type to be unchanged") {
                REQUIRE(std::is_same_v<stx::option_set<A, B, C>, decltype(set2)>);
            }
        }

        THEN("Merging in new types (rhs)") {
            auto set2 = set | D();

            THEN("Expecting the new option_set to contain type C") {
                REQUIRE(std::is_same_v<stx::option_set<A, B, C, D>, decltype(set2)>);
            }
        }

        THEN("Merging in new types (lhs)") {
            auto set2 = D() | set;

            THEN("Expecting the new option_set to contain type C") {
                REQUIRE(std::is_same_v<stx::option_set<D, A, B, C>, decltype(set2)>);
            }
        }

        THEN("Assigning the set to a fixed set variable") {
            auto fixed = stx::option_set<A, B, C>();
            THEN("Expecting all options to be unset") {
                REQUIRE(!fixed.has<A>());
                REQUIRE(!fixed.has<B>());
                REQUIRE(!fixed.has<C>());
            }

            fixed = set;
            THEN("Expecting all options to be set") {
                REQUIRE(fixed.has<A>());
                REQUIRE(fixed.has<B>());
                REQUIRE(fixed.has<C>());
            }
        }
    }

    GIVEN("An kwarg option set") {
        auto set = (kw::NAME_ = "Johannes") | (kw::AGE_ = 27);

        THEN("Expecting a set with two options") {
            REQUIRE(set.has<kw::NAME>());
            REQUIRE(set.has<kw::AGE>());

            THEN("Checking option values (==/!=)") {
                REQUIRE(*set.get<kw::NAME>() == "Johannes");
                REQUIRE(*set.get<kw::NAME>() != "Jochen");

                REQUIRE(*set.get<kw::AGE>() == 27);
                REQUIRE(*set.get<kw::AGE>() != 3);
            }

            THEN("Copy out option values") {
                std::string name = *set.get<kw::NAME>();
                (void)name;

                int age = *set.get<kw::AGE>();
                (void)age;
            }

            THEN("Merging a non-kwarg option") {
                auto set2 = set | A();

                REQUIRE(std::is_same_v<stx::option_set<kw::NAME, kw::AGE, A>, decltype(set2)>);
            }

            THEN("Passing set to a function") {
                REQUIRE(my_function(set) == "Johannes (27)");
            }

            THEN("Overwriting kwargs") {
                set |= kw::NAME_ = "Peter";
                set |= kw::AGE_ = 5;

                REQUIRE(my_function(set) == "Peter (5)");
            }
        }
    }
}
