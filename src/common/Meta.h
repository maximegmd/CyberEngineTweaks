#pragma once

// Source: https://stackoverflow.com/questions/4041447/how-is-stdtuple-implemented
template<std::size_t i, typename Item>
struct MetaArrayEntry
{
    Item value;
};

template<std::size_t i, typename... Items>
struct MetaArrayImpl;

template<std::size_t i>
struct MetaArrayImpl<i> {};

template<std::size_t i, typename HeadItem, typename... TailItems>
struct MetaArrayImpl<i, HeadItem, TailItems...> :
    MetaArrayEntry<i, HeadItem>,
    MetaArrayImpl<i + 1, TailItems...>
{};

template<typename... Items>
using MetaArray = MetaArrayImpl<0, Items...>;

// Source: https://www.reddit.com/r/cpp/comments/bhxx49/c20_string_literals_as_nontype_template/elwmdjp/
template<unsigned N>
struct FixedString {
    char buf[N + 1]{};
    constexpr FixedString(char const* s) {
        for (unsigned i = 0; i != N; ++i) buf[i] = s[i];
    }
    constexpr operator char const* () const { return buf; }
};
template<unsigned N> FixedString(char const (&)[N])->FixedString<N - 1>;