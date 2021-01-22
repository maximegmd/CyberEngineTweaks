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

#ifndef MEM_DEFINES_BRICK_H
#define MEM_DEFINES_BRICK_H

#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
#    define MEM_ARCH_X86_64
#elif defined(__i386) || defined(_M_IX86)
#    define MEM_ARCH_X86
#endif

#if defined(__AVX2__)
#    define MEM_SIMD_AVX2
#endif

#if defined(__AVX__) || defined(MEM_SIMD_AVX2)
#    define MEM_SIMD_AVX
#endif

#if defined(__SSSE3__) || defined(MEM_SIMD_AVX)
#    define MEM_SIMD_SSSE3
#endif

#if defined(__SSE3__) || defined(_M_AMD64) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP == 2)) || \
    defined(MEM_SIMD_SSSE3)
#    define MEM_SIMD_SSE3
#endif

#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64) || (defined(_M_IX86_FP) && (_M_IX86_FP == 2)) || \
    defined(MEM_SIMD_SSE3)
#    define MEM_SIMD_SSE2
#endif

#if defined(__SSE__) || (defined(_M_IX86_FP) && (_M_IX86_FP == 1)) || defined(MEM_SIMD_SSE2)
#    define MEM_SIMD_SSE
#endif

#if !defined(MEM_CONSTEXPR_14)
#    if (defined(__cpp_constexpr) && (__cpp_constexpr >= 201304)) || \
        (defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 191426433))
#        define MEM_CONSTEXPR_14 constexpr
#    else
#        define MEM_CONSTEXPR_14
#    endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#    define MEM_LIKELY(x) __builtin_expect(static_cast<bool>(x), 1)
#    define MEM_UNLIKELY(x) __builtin_expect(static_cast<bool>(x), 0)
#else
#    define MEM_LIKELY(x) static_cast<bool>(x)
#    define MEM_UNLIKELY(x) static_cast<bool>(x)
#endif

#if defined(__GNUC__) || defined(__clang__)
#    define MEM_STRONG_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#    define MEM_STRONG_INLINE __pragma(warning(suppress : 4714)) inline __forceinline
#else
#    define MEM_STRONG_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#    define MEM_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#    define MEM_NOINLINE __declspec(noinline)
#else
#    define MEM_NOINLINE
#endif

#include <climits>
#include <cstddef>
#include <cstdint>

#if CHAR_BIT != 8
#    error Only 8-bit bytes are supported
#endif

namespace mem
{
    using byte = unsigned char;

    inline namespace conventions
    {
        template <typename Result, typename... Args>
        using func_t = Result (*)(Args...);

        template <typename Result, typename Class, typename... Args>
        using member_func_t = Result (Class::*)(Args...);

#if defined(MEM_ARCH_X86)
#    if defined(__GNUC__) || defined(__clang__)
        template <typename Result, typename... Args>
        using cdecl_t = Result(__attribute__((cdecl)) *)(Args...);

        template <typename Result, typename... Args>
        using stdcall_t = Result(__attribute__((stdcall)) *)(Args...);

        template <typename Result, typename... Args>
        using fastcall_t = Result(__attribute__((fastcall)) *)(Args...);

        template <typename Result, typename... Args>
        using thiscall_t = Result(__attribute__((thiscall)) *)(Args...);
#    elif defined(_MSC_VER)
        template <typename Result, typename... Args>
        using cdecl_t = Result(__cdecl*)(Args...);

        template <typename Result, typename... Args>
        using stdcall_t = Result(__stdcall*)(Args...);

        template <typename Result, typename... Args>
        using fastcall_t = Result(__fastcall*)(Args...);

        template <typename Result, typename... Args>
        using thiscall_t = Result(__thiscall*)(Args...);
#    endif
#endif
    } // namespace conventions
} // namespace mem

#endif // MEM_DEFINES_BRICK_H
