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

#ifndef MEM_CHAR_QUEUE_BRICK_H
#define MEM_CHAR_QUEUE_BRICK_H

#include "defines.h"

#include <cstring>

namespace mem
{
    class char_queue
    {
    private:
        const char* start {nullptr};
        const char* end {nullptr};
        const char* current {nullptr};

    public:
        char_queue(const char* string);
        constexpr char_queue(const char* string, std::size_t length);

        constexpr int peek() const noexcept;

        MEM_CONSTEXPR_14 void pop() noexcept;
        constexpr std::size_t pos() const noexcept;

        constexpr explicit operator bool() const noexcept;
    };

    constexpr int xctoi(int value) noexcept;
    constexpr int dctoi(int value) noexcept;
    constexpr int octoi(int value) noexcept;

    MEM_STRONG_INLINE char_queue::char_queue(const char* string)
        : char_queue(string, std::strlen(string))
    {}

    MEM_STRONG_INLINE constexpr char_queue::char_queue(const char* string, std::size_t length)
        : start(string)
        , end(start + length)
        , current(start)
    {}

    MEM_STRONG_INLINE constexpr int char_queue::peek() const noexcept
    {
        return (current < end) ? static_cast<byte>(*current) : -1;
    }

    MEM_STRONG_INLINE MEM_CONSTEXPR_14 void char_queue::pop() noexcept
    {
        if (current < end)
        {
            ++current;
        }
    }

    MEM_STRONG_INLINE constexpr std::size_t char_queue::pos() const noexcept
    {
        return static_cast<std::size_t>(current - start);
    }

    MEM_STRONG_INLINE constexpr char_queue::operator bool() const noexcept
    {
        return current < end;
    }

    MEM_STRONG_INLINE constexpr int xctoi(int value) noexcept
    { // clang-format off
        return (value >= '0' && value <= '9') ? (value - '0')
             : (value >= 'a' && value <= 'f') ? (value - 'a' + 10)
             : (value >= 'A' && value <= 'F') ? (value - 'A' + 10)
             : -1;
    } // clang-format on

    MEM_STRONG_INLINE constexpr int dctoi(int value) noexcept
    {
        return (value >= '0' && value <= '9') ? (value - '0') : -1;
    }

    MEM_STRONG_INLINE constexpr int octoi(int value) noexcept
    {
        return (value >= '0' && value <= '7') ? (value - '0') : -1;
    }
} // namespace mem

#endif // MEM_CHAR_QUEUE_BRICK_H
