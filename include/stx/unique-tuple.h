#pragma once

#include <tuple>
#include <type_traits>

namespace stx
{

template <class _Head, class...>
struct unique
{
    using type = _Head;
};

template <class... _Types, class _Head, class... _Tail>
struct unique<std::tuple<_Types...>, _Head, _Tail...>
    : std::conditional_t<(std::is_same_v<_Head, _Tail> || ...),          /* If head in tail. */
                         unique<std::tuple<_Types...>, _Tail...>,        /* Skip head. */
                         unique<std::tuple<_Types..., _Head>, _Tail...>> /* Append head. */
{};

template <class... _Types>
using unique_tuple = typename unique<std::tuple<>, _Types...>::type;

}
