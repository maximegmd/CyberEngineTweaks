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

#ifndef MEM_HASHER_BRICK_H
#define MEM_HASHER_BRICK_H

#include "defines.h"

namespace mem
{
    class hasher
    {
    private:
        std::uint32_t hash_;

    public:
        hasher(std::uint32_t seed = 0) noexcept;

        void update(const void* data, std::size_t length) noexcept;

        template <typename T>
        void update(const T& value) noexcept;

        std::uint32_t digest() const noexcept;
    };

    MEM_STRONG_INLINE hasher::hasher(std::uint32_t seed) noexcept
        : hash_(seed)
    {}

    MEM_STRONG_INLINE void hasher::update(const void* data, std::size_t length) noexcept
    {
        std::uint32_t hash = hash_;

        for (std::size_t i = 0; i < length; ++i)
        {
            hash += static_cast<const std::uint8_t*>(data)[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }

        hash_ = hash;
    }

    template <typename T>
    MEM_STRONG_INLINE void hasher::update(const T& value) noexcept
    {
        static_assert(std::is_integral<T>::value, "Invalid Type");

        update(&value, sizeof(value));
    }

    MEM_STRONG_INLINE std::uint32_t hasher::digest() const noexcept
    {
        std::uint32_t hash = hash_;

        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        return hash;
    }
} // namespace mem

#endif // MEM_HASHER_BRICK_H
