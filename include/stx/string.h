#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <type_traits>
#include <iterator>

/* For optional Qt -> std conversions */
#if defined(QT_CORE_LIB)
#include <QString>
#include <QUuid>
#endif

namespace stx
{

/**
 * Split string `what` at `at`, return list of parts.
 *
 * Note: Use `container<std::string_view>`, for zero-copy (string) split.
 *       If used, `what` _must_ outlive the result!
 */
template <class _Container = std::vector<std::string>>
_Container split(std::string_view what,
                 std::string_view at,
                 bool removeEmpty = true)
{
    using ResultType = typename _Container::value_type;

    _Container container;
    auto out = std::back_inserter(container);

    /* Special case: empty `what` */
    if (what.empty())
        return container;

    /* Special case: empty `at` */
    if (at.empty()) {
        *out++ = ResultType(what);
        return container;
    }

    auto begin = 0ull;
    auto end = 0ull;

    auto next = [&]() {
        if ((end = what.find(at, begin)) != std::string::npos) {
            if (end - begin || !removeEmpty)
                *out++ = ResultType(what).substr(begin, end - begin);

            begin = end + at.size();
            return true;
        }

        if (what.size() - begin || !removeEmpty)
            *out++ = ResultType(what).substr(begin);

        return false;
    };

    while (next()) { /* noop */ }

    return container;
}

template <class _Iter>
std::string join(_Iter begin, _Iter end, const std::string& with)
{
    if (begin == end)
        return {};

    std::size_t withSize = (std::min<std::size_t>(std::distance(begin, end), 1u) - 1u) * with.size();
    std::size_t size = std::accumulate(begin, end, 0ull, [](auto init, const auto& v) {
        return init + v.size();
    });

    std::string result;
    result.reserve(size + withSize);

    std::for_each(begin, end, [&, i = 0u](const auto& v) mutable {
        if (i++ > 0u)
            result += with;
        result += v;
    });

    return result;
}

namespace impl
{

template <class>
struct from_string;

template <>
struct from_string<int>
{
    static auto from(const std::string& v)
    {
        return std::atoi(v.data());
    }
};

template <>
struct from_string<float>
{
    static auto from(const std::string& v)
    {
        return std::atof(v.data());
    }
};

template <>
struct from_string<std::string>
{
    static auto from(const std::string& v)
    {
        return v;
    }
};

#if defined(QT_CORE_LIB)
template <>
struct from_string<QString>
{
    static auto from(const std::string& v)
    {
        return QString::fromStdString(v);
    }
};

template <>
struct from_string<QUuid>
{
    static auto from(const std::string& v)
    {
        return QUuid::fromString(QString::fromStdString(v));
    }
};
#endif

}

/**
 * Convert string to T.
 */
template <class _T>
_T from_string(const std::string& v)
{
    return impl::from_string<_T>::from(v);
}


namespace impl
{

template <class>
struct to_string;

template <class _T>
struct to_string
{
    static auto to(const _T& v)
    {
        return std::to_string(v);
    }
};

template <>
struct to_string<const char*>
{
    static auto to(const char* v)
    {
        return std::string(v);
    }
};

template <>
struct to_string<std::string>
{
    static auto to(std::string v)
    {
        return v;
    }
};

template <>
struct to_string<std::string_view>
{
    static auto to(std::string_view v)
    {
        return std::string(v);
    }
};

#if defined(QT_CORE_LIB)
template <>
struct to_string<QString>
{
    static auto to(const QString& v)
    {
        return v.toStdString();
    }
};

template <>
struct to_string<QUuid>
{
    static auto to(const QUuid& v)
    {
        return v.toString(QUuid::WithoutBraces).toStdString();
    }
};
#endif

}

/**
 * Convert T to string.
 *
 * Because there is no std::to_string with std::string as parameter type,
 * this helper function is needed to use std::to_string with a generic input type.
 */
template <class _T>
std::string to_string(_T&& v)
{
    return impl::to_string<std::decay_t<_T>>::to(std::forward<_T>(v));
}

/**
 *
 */

namespace impl
{

template <std::size_t _Index1>
struct get_as_string
{
    static constexpr std::size_t Index0 = _Index1 - 1u;

    template <class... _Types>
    static std::string get(const std::size_t index,
                           const std::tuple<_Types...>& tuple)
    {
        return index == Index0
            ? stx::to_string(std::get<Index0>(tuple))
            : get_as_string<_Index1 - 1u>::get(index, tuple);
    }
};


template <>
struct get_as_string<0u>
{
    template <class... _Types>
    static std::string get(const std::size_t,
                           const std::tuple<_Types...>&)
    {
        return {};
    }
};

}

/**
 * Runtime std::get<> alternative, returning the value as string (using to_string).
 */
template <class _Tuple>
std::string get_as_string(const std::size_t index,
                          const _Tuple& values)
{
    return impl::get_as_string<std::tuple_size_v<_Tuple>>::get(index, values);
}

/**
 * Expand placeholder `placeholder` in string `source`
 * with values (++) from value list `values`.
 *
 * E.g.: "We all live in a ? ?" with ("yellow", "submarine") returns
 *       "We all live in a yellow submarine"
 */
template <class... _Args>
std::string replace_with(std::string source,
                         const std::string& what,
                         _Args&& ...with)
{
    auto tuple = std::tie(with...);

    auto find_next = [&](auto start) {
        start = source.find(what, start);
        if (start != std::string::npos) {
            return start;
        }

        return std::string::npos;
    };

    std::size_t index = 0u;
    std::string::size_type pos = 0u;
    while ((pos = find_next(pos)) != std::string::npos) {
        auto replacement = get_as_string(index++, tuple);
        source.replace(pos, what.size(), replacement);
        pos += replacement.size();
    }

    return source;
}

template <class _Iter>
std::string to_hex(_Iter begin, _Iter end, bool upcase = false)
{
    const char* charset[2] = {"0123456789abcdef", "0123456789ABCDEF"};

    using ValueType = std::remove_reference_t<decltype(*begin)>;
    static_assert(std::is_integral_v<ValueType>);
    const auto type_size = sizeof(ValueType);

    std::string str;
    str.reserve(2u * std::distance(begin, end) * type_size);
    std::for_each(begin, end, [&](const auto v) {
        auto i = type_size - 1u;
        do {
            const auto& byte = ((const std::uint8_t*)&v)[i];

            str.push_back(charset[upcase ? 1 : 0][byte >> 4]);
            str.push_back(charset[upcase ? 1 : 0][byte & 0xf]);
        } while (i--);
    });
    return str;
}

}
