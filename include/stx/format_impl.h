#pragma once

#include <string_view>
#include <type_traits>
#include <optional>

#include "formatter.h"
#include "format_charconv_impl.h"

namespace stx::format_impl
{

/**
 * Helper for calling formatter<T>::format on the n-the tuple element.
 */
template <size_t>
struct format_value_at_t;

template <size_t _Index>
struct format_value_at_t
{
    /* NOTE: _Index is 1based, Index0 is 0based */
    static constexpr auto Index0 = _Index - 1u;

    template <class _Tuple, class _Iter>
    static void format(size_t index, const _Tuple& t, std::string_view fmt, _Iter out)
    {
        if (index == Index0) {
            formatter<std::decay_t<std::tuple_element_t<Index0, _Tuple>>> vf(fmt);
            return vf.format(std::get<Index0>(t), out);
        }

        return format_value_at_t<_Index - 1 /* 1based */>::format(index, t, fmt, out);
    }
};

template <>
struct format_value_at_t<0u>
{
    template <class _Tuple, class _Iter>
    static void format(size_t, const _Tuple&, std::string_view, _Iter)
    {}
};

template <class _Tuple, class _Iter>
void format_value_at(size_t index, const _Tuple& t, std::string_view fmt, _Iter out)
{
    format_value_at_t<std::tuple_size_v<_Tuple>>::format(index, t, fmt, out);
}

}
