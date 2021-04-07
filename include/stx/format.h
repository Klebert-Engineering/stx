#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <cstring>
#include <algorithm>

#include "formatter.h"
#include "format_impl.h"
#include "format_charconv_impl.h"

namespace stx
{

/**
 * See `format`.
 */
template <class _Iter, class... _Args>
_Iter format_to(_Iter out, std::string_view fmt, const _Args& ...args)
{
    std::tuple<const _Args&...> tuple(args...);

    auto replace = [&tuple, next = 0u](auto begin, auto end, auto out) mutable {
        std::size_t index = 0u;
        std::string_view format;

        if (begin == end)
            return;

        /* Parse optional argument index */
        if (auto [iter, ok] = format_impl::parse_int(begin, end, index); ok) {
            begin = iter;
        } else {
            index = next++; /* Auto incr. index */
        }

        /* Parse optional argument format */
        if (begin != end && *begin == ':') {
            if (++begin != end) {
                const auto format_len = std::max<size_t>(std::distance(begin, end), 1) - 1; /* end is +1 off (behind the closing brace) */
                format = std::string_view(&(*begin), static_cast<std::string_view::size_type>(format_len));
            }
        }

        format_impl::format_value_at(index, tuple, format, out);
    };

    /* NOTE: Nested braces are not (yet?) allowed. */
    auto skip_format = [](auto begin, auto end) {
        while (begin != end) {
            if (*begin == '}') {
                if (++begin == end)
                    return begin;

                /* Escape double r-brace. */
                if (*begin != '}')
                    return begin;
            }

            ++begin;
        }

        return begin;
    };

    auto begin = fmt.cbegin();
    auto end = fmt.cend();

    while (begin != end) {
        if (*begin == '{') {
            if (++begin == end)
                break;

            /* Escape double l-brace. */
            if (*begin == '{') {
                *out = *begin++;
            } else {
                auto fmt_end = skip_format(begin, end);
                replace(begin, fmt_end, out);

                begin = fmt_end;
            }
        } else if (*begin == '}') {
            if (++begin == end)
                break;

            /* Escape double r-brace. */
            if (*begin == '}') {
                *out++ = *begin++;
            }
        } else {
            *out++ = *begin++;
        }
    }

    return out;
}

/**
 * Safely format strings.
 *
 * @param fmt   Format string – see examples.
 * @param args  Values to format.
 * @return  The formatted string.
 *
 * See `format_to` for a version that uses an output iterator.
 *
 * Basic examples:
 *   format("{}", 123) => "123"
 *   format("{}, {}, {}", 1, "a", true) => "1, a, true"
 *
 * Index example:
 *   format("{1} {0}", "a", 1) => "1 a"
 *
 * Format example:
 *   By default, numbers are justified right, strings are justified left
 *     format("{:5} {:5}", 123, "xx") => "  123 xx   "
 *
 *   Justification can be specified with <, > and ^ (left, right, center).
 *   The fill-char can be specified as first argument.
 *      format("{:0>5} {:_<6} {:.^3}", 123, "xx", 0) => "00123 xx____ .0."
 *
 * Nested {} as std::format supports them are not supported!
 */
template <class... _Args>
std::string format(std::string_view fmt, const _Args& ...args)
{
    std::string out;
    out.reserve(fmt.size());

    format_to(std::back_inserter(out), fmt, args...);

    return out;
}

}
