#pragma once

// Source: https://www.reddit.com/r/cpp/comments/bhxx49/c20_string_literals_as_nontype_template/elwmdjp/
template <unsigned N> struct FixedString
{
    char buf[N + 1]{};
    constexpr FixedString(char const* s)
    {
        for (unsigned i = 0; i != N; ++i)
            buf[i] = s[i];
    }
    constexpr operator char const*() const { return buf; }
};
template <unsigned N> FixedString(char const (&)[N]) -> FixedString<N - 1>;