#pragma once

#include <optional>
#include <tuple>
#include <type_traits>

namespace stx
{

template <class...>
struct option_set;

namespace impl
{

template <class>
struct is_option_set : std::false_type {};

template <class... _Types>
struct is_option_set<option_set<_Types...>> : std::true_type {};

template <class _Type>
struct remove_optional
{
    using type = _Type;
};

template <class _Type>
struct remove_optional<std::optional<_Type>>
{
    using type = _Type;
};

template <class _Type>
using remove_optional_t = typename remove_optional<_Type>::type;

template <class _Head, class...>
struct unique_option_set
{
    using type = _Head;
};

template <class... _Types, class _Head, class... _Tail>
struct unique_option_set<option_set<_Types...>, _Head, _Tail...>
{
    using type = std::conditional_t<(std::is_same_v<_Head, _Types> || ...),
                                    typename unique_option_set<option_set<_Types...>, _Tail...>::type,
                                    typename unique_option_set<option_set<_Types..., _Head>, _Tail...>::type>;
};

template <class... _Types>
using unique_option_set_t = typename unique_option_set<option_set<>, _Types...>::type;

template <class, class>
struct merged_option_set;

template <class... _First, class... _Second>
struct merged_option_set<option_set<_First...>, option_set<_Second...>>
{
    using type = unique_option_set_t<remove_optional_t<_First>...,
                                     remove_optional_t<_Second>...>;
};

template <class _Lhs, class _Rhs>
using merged_option_set_t = typename merged_option_set<_Lhs, _Rhs>::type;

}

template <class... _Types>
struct option_set : std::tuple<std::optional<_Types>...>
{
    option_set()
    {}

    template <class _Option,
              class _Enable = std::enable_if_t<!impl::is_option_set<_Option>::value>>
    option_set(_Option option)
    {
        set<_Option>(std::move(option));
    }

    template <class... _Options>
    option_set(option_set<_Options...> rhs)
    {
        set(std::move(rhs));
    }

    option_set(const option_set& rhs)
    {
        set(rhs);
    }

    auto set(const option_set& rhs) -> option_set&
    {
        return *this = rhs;
    }

    template <class _Option, class _Enable = std::enable_if_t<!impl::is_option_set<_Option>::value>>
    auto set(_Option rhs) -> option_set&
    {
        std::get<std::optional<std::decay_t<_Option>>>(*this) =
            std::move(rhs);
        return *this;
    }

    template <class... _RTypes>
    auto set(option_set<_RTypes...>&& rhs) -> option_set&
    {
        (void)((std::get<std::optional<_RTypes>>(*this) =
                std::move(std::get<std::optional<_RTypes>>(rhs))) && ...);
        return *this;
    }

    template <class _Option>
    auto has() const -> bool
    {
        return std::get<std::optional<_Option>>(*this) != std::nullopt;
    }

    template <class _Option>
    auto get() const -> const _Option*
    {
        if (decltype(auto) option = std::get<std::optional<_Option>>(*this))
            return &*option;
        return nullptr;
    }

    /* Operators */

    template <class... _RTypes>
    auto operator|=(option_set<_RTypes...> rhs) -> option_set&
    {
        return set(std::move(rhs));
    }

    template <class _Option>
    auto operator|=(_Option&& rhs) -> option_set&
    {
        return set(std::forward<_Option>(rhs));
    }

    template <class... _RTypes>
    auto operator|(option_set<_RTypes...> rhs) const &
    {
        return impl::merged_option_set_t<option_set, option_set<_RTypes...>>()
            .set(*this)
            .set(std::move(rhs));
    }

    template <class... _RTypes>
    auto operator|(option_set<_RTypes...> rhs) &&
    {
        return impl::merged_option_set_t<option_set, option_set<_RTypes...>>()
            .set(std::move(*this))
            .set(std::move(rhs));
    }

    template <class _Option,
              class _Enable = std::enable_if_t<!impl::is_option_set<_Option>::value>>
    auto operator|(_Option rhs) const &
    {
        return impl::merged_option_set_t<option_set, option_set<std::decay_t<_Option>>>()
            .set(*this)
            .template set<_Option>(std::move(rhs));
    }

    template <class _Option,
              class _Enable = std::enable_if_t<!impl::is_option_set<_Option>::value>>
    auto operator|(_Option rhs) &&
    {
        return impl::merged_option_set_t<option_set, option_set<std::decay_t<_Option>>>()
            .set(std::move(*this))
            .template set<_Option>(std::move(rhs));
    }
};

/**
 * Declare option type concat (|) operators.
 *
 * Example:
 *   struct MyOption {
 *     STX_OPTION_OPERATORS()
 *
 *     bool force = false;
 *     bool verbose = false;
 *   };
 *
 *   my_function(OtherOption() | MyOption());
 */
#define STX_OPTION_OPERATORS()                                          \
    template <class _Other>                                             \
    auto operator|(_Other rhs) const                                    \
    {                                                                   \
        return stx::option_set<std::decay_t<decltype(*this)>, _Other>() \
            .set(*this) | std::move(rhs);                               \
    }                                                                   \
                                                                        \
    template <class... _RTypes>                                         \
    auto operator|(stx::option_set<_RTypes...> rhs) const               \
    {                                                                   \
        return stx::option_set<std::decay_t<decltype(*this)>>()         \
            .set(*this) | std::move(rhs);                               \
    }

/**
 * Declare a simple (on/off) option type.
 *
 * Example:
 *   STX_DECLARE_FLAG_OPTION(FollowSymlinks)
 *   STX_DECLARE_FLAG_OPTION(Recursive)
 *
 *   my_function(FollowSymlinks() | Recursive())
 */
#define STX_DECLARE_FLAG_OPTION(ident)          \
    struct ident { STX_OPTION_OPERATORS() };

namespace option_kw
{

template <class _Type, class>
struct kwarg
{
    struct option_type
    {
        _Type value;

        operator _Type&() {return value;}
        operator const _Type&() const {return value;}

        auto operator==(const _Type& rhs) const { return value == rhs; }
        auto operator!=(const _Type& rhs) const { return value != rhs; }

        STX_OPTION_OPERATORS();
    };

    option_type operator=(_Type value) const
    {
        return option_type{std::move(value)};
    }
};

/**
 * Declare keyword-argument like option type.
 *
 * Parameters:
 *   ident  Unique type identifier
 *   type   Value type
 *
 * Creates a unique type `ident` and a static constant variable indent + '_'.
 *
 * Example:
 *   STX_DECLARE_KWARG(Name);
 *   STX_DECLARE_KWARG(Age);
 *
 *   my_function((Name_ = "Johannes") | (Age_ = 27)); // <- Note the '_' for assigning kwargs.
 *   ...
 *   { args.get<Name>() ... } // <- Use the ident (without '_') for access.
 */
#define STX_DECLARE_KWARG(ident, type)                                                  \
    static const stx::option_kw::kwarg<type, struct ident ## __uniqkwarg__> ident ## _; \
    using ident = typename decltype(ident ## _)::option_type;                           \

}

}
