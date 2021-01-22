/*
    Copyright 2018 Brick

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute,
    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MEM_BITWISE_ENUM_BRICK_H
#define MEM_BITWISE_ENUM_BRICK_H

#include "defines.h"

#include <type_traits>

#define MEM_DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)                                              \
    MEM_STRONG_INLINE constexpr ENUMTYPE operator|(ENUMTYPE a, ENUMTYPE b) noexcept           \
    {                                                                                         \
        return static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(a) |   \
            static_cast<std::underlying_type<ENUMTYPE>::type>(b));                            \
    }                                                                                         \
    MEM_STRONG_INLINE MEM_CONSTEXPR_14 ENUMTYPE& operator|=(ENUMTYPE& a, ENUMTYPE b) noexcept \
    {                                                                                         \
        return a = a | b;                                                                     \
    }                                                                                         \
    MEM_STRONG_INLINE constexpr ENUMTYPE operator&(ENUMTYPE a, ENUMTYPE b) noexcept           \
    {                                                                                         \
        return static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(a) &   \
            static_cast<std::underlying_type<ENUMTYPE>::type>(b));                            \
    }                                                                                         \
    MEM_STRONG_INLINE MEM_CONSTEXPR_14 ENUMTYPE& operator&=(ENUMTYPE& a, ENUMTYPE b) noexcept \
    {                                                                                         \
        return a = a & b;                                                                     \
    }                                                                                         \
    MEM_STRONG_INLINE constexpr ENUMTYPE operator^(ENUMTYPE a, ENUMTYPE b) noexcept           \
    {                                                                                         \
        return static_cast<ENUMTYPE>(static_cast<std::underlying_type<ENUMTYPE>::type>(a) ^   \
            static_cast<std::underlying_type<ENUMTYPE>::type>(b));                            \
    }                                                                                         \
    MEM_STRONG_INLINE MEM_CONSTEXPR_14 ENUMTYPE& operator^=(ENUMTYPE& a, ENUMTYPE b) noexcept \
    {                                                                                         \
        return a = a ^ b;                                                                     \
    }                                                                                         \
    MEM_STRONG_INLINE constexpr ENUMTYPE operator~(ENUMTYPE a) noexcept                       \
    {                                                                                         \
        return static_cast<ENUMTYPE>(~static_cast<std::underlying_type<ENUMTYPE>::type>(a));  \
    }

#endif // MEM_BITWISE_ENUM_BRICK_H
