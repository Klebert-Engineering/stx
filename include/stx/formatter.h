#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <algorithm>
#include <type_traits>
#include <array>

#include "string.h"
#include "format_charconv_impl.h"

namespace stx
{

/**
 * Non-virtual formatter base for sharing functionality.
 */
struct formatter_base
{
    std::optional<char> fillc;
    std::optional<char> justify;
    std::optional<size_t> min_width;

    /**
     * Eat the first char from string_view `sv` if contained in `in`
     * or if `in` is unset.
    */
    static std::optional<char> eat_char(std::string_view& sv, std::string_view in)
    {
        if (sv.empty())
            return {};

        auto c = sv.front();
        if (!in.empty() && in.find(c) == std::string_view::npos)
            return {};

        sv.remove_prefix(1);
        return c;
    }

    /**
     * Eat the first number of type _T from string_view `sv`
     */
    template <class _T>
    static std::optional<_T> eat_number(std::string_view& sv)
    {
        if (sv.empty())
            return {};

        _T v;
        auto [iter, ok] = format_impl::parse_int(sv.begin(), sv.end(), v);
        if (!ok)
            return {};

        sv.remove_prefix(std::distance(sv.begin(), iter));
        return v;
    }

    /**
     * Formatter base ctor
     *
     * Parsing all generic/universal formatting options.
     * Child classes must call justify_pre and justify_post
     * before and after writing their values.
     */
    formatter_base(std::string_view& fmt)
    {
        /* {:[.][<>^][0-9]...} */
        if (fmt.find_first_of("<^>") == 1u) {
            fillc = eat_char(fmt, {});
            justify = eat_char(fmt, "<^>");
        /* {:[<>^][0-9]} */
        } else if (fmt.find_first_of("<^>") == 0u) {
            justify = eat_char(fmt, "<^>");
        }

        /* {:[0-9]} */
        min_width = eat_number<unsigned>(fmt);

        /* {:[.]} */
        if (!fillc && !min_width)
            fillc = eat_char(fmt, {});
    }

    template <class _Iter>
    void justify_pre(size_t width, _Iter out)
    {
        if (justify == '>') {
            if (width < min_width)
                std::fill_n(out, min_width.value_or(width) - width, fillc.value_or(' '));
        } else if (justify == '^') {
            if (width < min_width)
                std::fill_n(out, (min_width.value_or(width) - width) / 2, fillc.value_or(' '));
        } else if (justify == '<') {
            /* Special case: Prepend 1 fillc if value is non-empty.
             * Why? This supports a handy trick to prepend a space to non-empty values.
             * E.g. format("Hello{: }.", name) -> "Hello." or "Hello Johannes." */
            if (!min_width && fillc && width > 0)
                *out++ = *fillc;
        }
    }

    template <class _Iter>
    void justify_post(size_t width, _Iter out)
    {
        if (justify == '<') {
            if (width < min_width)
                std::fill_n(out, min_width.value_or(width) - width, fillc.value_or(' '));
        } else if (justify == '^') {
            if (width < min_width)
                std::fill_n(out, (min_width.value_or(width) - width) / 2 + ((min_width.value_or(width) - width) % 2),
                            fillc.value_or(' '));
        } else if (justify == '>') {
            /* See `justify_pre` comment. */
            if (!min_width && fillc && width > 0)
                *out++ = *fillc;
        }
    }
};

/**
 * Type formatters.
 */
template <class, class _Enable = void>
struct formatter;

template <>
struct formatter<bool> : formatter_base
{
    using formatter_base::formatter_base;

    template <class _Iter>
    void format(bool value, _Iter out)
    {
        static const char* s[] = {"false", "true"};
        const auto size = value ? 4 : 5;

        justify_pre(size, out);
        std::copy_n(s[!!value], size, out);
        justify_post(size, out);
    }
};

template <class _Type>
struct formatter<_Type, std::enable_if_t<std::is_floating_point_v<_Type>>> : formatter_base
{
    using formatter_base::formatter_base;

    formatter(std::string_view& sv)
        : formatter_base(sv)
    {
        if (!justify)
            justify = '>';
    }

    template <class _Iter>
    void format(_Type value, _Iter out)
    {
        char buffer[30];
        buffer[0] = '\0';

        auto size = std::snprintf(buffer, sizeof(buffer), "%g",  value);

        justify_pre(size, out);
        std::copy_n(buffer, size, out);
        justify_post(size, out);
    }
};


template <class _Type>
struct formatter<_Type, std::enable_if_t<std::is_integral_v<_Type>>> : formatter_base
{
    int base = 10;

    formatter(std::string_view& sv)
        : formatter_base(sv)
    {
        if (!justify)
            justify = '>';

        auto base_ident = formatter_base::eat_char(sv, "xdo").value_or('d');
        base = base_ident == 'x' ? 16 :
               base_ident == 'd' ? 10 :
               base_ident == 'o' ? 8 : 10;
    }

    template <class _Iter>
    void format(_Type value, _Iter out)
    {
        using MaxType = std::conditional_t<std::is_signed_v<_Type>, std::intmax_t, std::uintmax_t>;
        const MaxType maxValue = static_cast<MaxType>(value);
        auto uabs = [](auto value) -> std::uintmax_t {
            if constexpr (std::is_signed_v<_Type>)
                return (std::uintmax_t)std::abs((std::intmax_t)value);
            else
                return (std::uintmax_t)value;
        };

        char buffer[30];
        buffer[0] = '\0';

        std::size_t size = 0u;
        switch (base) {
        case 8:
            size = std::snprintf(buffer, sizeof(buffer), "%s%jo",
                                 maxValue < 0 ? "-" : "", uabs(maxValue));
            break;
        case 16:
            size = std::snprintf(buffer, sizeof(buffer), "%s%jx",
                                 maxValue < 0 ? "-" : "", uabs(maxValue));
            break;
        default:
            if constexpr (std::is_signed_v<_Type>)
                size = std::snprintf(buffer, sizeof(buffer), "%jd",
                                     maxValue);
            else
                size = std::snprintf(buffer, sizeof(buffer), "%ju",
                                     maxValue);
            break;
        }

        justify_pre(size, out);
        std::copy_n(buffer, size, out);
        justify_post(size, out);
    }
};

template <class _Type>
struct formatter<_Type, std::enable_if_t<std::is_same_v<_Type, std::string_view> ||
                                         std::is_same_v<_Type, std::string>>> : formatter_base
{
    formatter(std::string_view& sv)
        : formatter_base(sv)
    {
        if (!justify)
            justify = '<';
    }

    template <class _Iter>
    void format(const _Type& value, _Iter out)
    {
        justify_pre(value.size(), out);
        std::copy(value.begin(), value.end(), out);
        justify_post(value.size(), out);
    }
};

template <>
struct formatter<const char*> : formatter<std::string_view>
{
    using formatter<std::string_view>::formatter;

    template <class _Iter>
    void format(const char* value, _Iter out)
    {
        return formatter<std::string_view>::format(value, out);
    }
};

#if defined(QT_CORE_LIB)
template <class _Type>
struct formatter<_Type, std::enable_if_t<std::is_same_v<_Type, QString> ||
                                         std::is_same_v<_Type, QUuid>>> : formatter<std::string_view>
{
    using formatter<std::string_view>::formatter;

    template <class _Iter>
    void format(const _Type& value, _Iter out)
    {
        return formatter<std::string_view>::format(stx::to_string(value), out);
    }
};
#endif

}
