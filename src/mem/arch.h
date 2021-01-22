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

#ifndef MEM_ARCH_BRICK_H
#define MEM_ARCH_BRICK_H

#include "defines.h"

#if defined(MEM_ARCH_X86) || defined(MEM_ARCH_X86_64)
#    if defined(_MSC_VER)
#        include <intrin.h>
#        pragma intrinsic(__rdtsc)
#        pragma intrinsic(_BitScanForward)
#    else
#        include <x86intrin.h>
#    endif
#endif

namespace mem
{
#if defined(MEM_ARCH_X86) || defined(MEM_ARCH_X86_64)
    MEM_STRONG_INLINE std::uint64_t rdtsc() noexcept
    {
        return __rdtsc();
    }

    MEM_STRONG_INLINE unsigned int bsf(unsigned int x) noexcept
    {
#    if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))
        return static_cast<unsigned int>(__builtin_ctz(x));
#    elif defined(_MSC_VER)
        unsigned long result;
        _BitScanForward(&result, static_cast<unsigned long>(x));
        return static_cast<unsigned int>(result);
#    else
        unsigned int result;
        asm("bsf %1, %0" : "=r"(result) : "rm"(x));
        return result;
#    endif
    }
#endif
} // namespace mem

#endif // MEM_ARCH_BRICK_H
