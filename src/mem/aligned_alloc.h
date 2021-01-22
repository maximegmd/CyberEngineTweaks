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

#ifndef MEM_ALIGNED_ALLOC_BRICK_H
#define MEM_ALIGNED_ALLOC_BRICK_H

#include "defines.h"

#include <memory>

#if defined(_WIN32)
#    include <malloc.h>
#else
#    include <cstdlib>
#endif

namespace mem
{
    void* aligned_alloc(std::size_t size, std::size_t alignment);
    void aligned_free(void* address);

    MEM_STRONG_INLINE void* aligned_alloc(std::size_t size, std::size_t alignment)
    {
#if defined(_WIN32)
        return _aligned_malloc(size, alignment);
#elif (_POSIX_C_SOURCE >= 200112L)
        void* result = nullptr;

        if (posix_memalign(&result, size, alignment) != 0)
        {
            return nullptr;
        }

        return result;
#else
        const std::size_t max_offset = alignment - 1 + sizeof(void*);
        void* result = std::malloc(size + max_offset);

        if (result)
        {
            void* aligned_result = reinterpret_cast<void*>(
                (reinterpret_cast<std::uintptr_t>(result) + max_offset) / alignment * alignment);
            reinterpret_cast<void**>(aligned_result)[-1] = result;
            result = aligned_result;
        }

        return result;
#endif
    }

    MEM_STRONG_INLINE void aligned_free(void* address)
    {
        if (!address)
        {
            return;
        }

#if defined(_WIN32)
        _aligned_free(address);
#elif (_POSIX_C_SOURCE >= 200112L)
        free(address);
#else
        std::free(reinterpret_cast<void**>(address)[-1]);
#endif
    }
} // namespace mem

#endif // MEM_ALIGNED_ALLOC_BRICK_H
