#pragma once

#include <map>
#include <unordered_map>

#include <type_traits>

namespace stx
{

/**
 * Returns value for key `k`, or fallback value `v` if no value for
 * the key could be found.
 *
 * Rvalue overload for always copying result value, if map is an rvalue.
 */
template <template <class, class, class...> class _C, class _KeyType, class _ValueType, class... _Args, class _FallbackType>
auto value_or(_C<_KeyType, _ValueType, _Args...>&& c,
              const _KeyType& k,
              _FallbackType v)
    -> _ValueType
{
    auto iter = c.find(k);
    if (iter != c.end())
        return iter->second;
    return v;
}

/**
 * Returns value for key `k`, or fallback value `v` if no value for
 * the key could be found.
 */
template <template <class, class, class...> class _C, class _KeyType, class _ValueType, class... _Args, class _FallbackType>
auto value_or(const _C<_KeyType, _ValueType, _Args...>& c,
              const _KeyType& k,
              _FallbackType&& v)
    -> std::conditional_t<std::is_lvalue_reference_v<_FallbackType>, const _ValueType&, _ValueType>
{
    auto iter = c.find(k);
    if (iter != c.end())
        return iter->second;
    return std::forward<_FallbackType>(v);
}

}
