#pragma once

namespace stx::format_impl
{

/**
 * Parse an base-10 unsigned integer from range `begin` to `end`.
 *
 * Returns the iterator to the first non matching character, or `end`.
 */
template <class _Iter, class _Int>
std::pair<_Iter, bool> parse_int(_Iter begin, _Iter end, _Int& out)
{
    static_assert(std::is_integral_v<_Int> && std::is_unsigned_v<_Int>,
                  "Result type must be an unsigned integer type");

    out = 0;

    auto valid = false;
    while (begin != end) {
        if (*begin >= '0' && *begin <= '9') {
            valid = true;
            out *= 10;
            out += *begin - '0';
            ++begin;
        } else {
            break;
        }
    }

    return {begin, valid};
}

}
