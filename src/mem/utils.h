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

#ifndef MEM_UTILS_BRICK_H
#define MEM_UTILS_BRICK_H

#include "mem.h"

#include <string>
#include <vector>

#include "char_queue.h"

namespace mem
{
    bool is_ascii(region range) noexcept;
    bool is_utf8(region range) noexcept;

    std::string as_string(region range);
    std::string as_hex(region range, bool upper_case = true, bool padded = true);

    std::vector<byte> unescape(const char* string, std::size_t length, bool strict = false);

    MEM_STRONG_INLINE bool is_ascii(region range) noexcept
    {
        for (std::size_t i = 0; i < range.size; ++i)
        {
            if (range.start.at<const byte>(i) >= 0x80)
            {
                return false;
            }
        }

        return true;
    }

    inline bool is_utf8(region range) noexcept
    {
        // clang-format off
        static constexpr const std::uint8_t utf8_length_table[256]
        {
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
            3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
            4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,
        };
        // clang-format on

        for (std::size_t i = 0; i < range.size;)
        {
            const std::size_t length = utf8_length_table[range.start.at<const byte>(i)];

            if (length == 0)
            {
                return false;
            }

            if ((i + length) > range.size)
            {
                return false;
            }

            for (std::size_t j = 1; j < length; ++j)
            {
                if ((range.start.at<const byte>(i + j) & 0xC0) != 0x80)
                {
                    return false;
                }
            }

            i += length;
        }

        return true;
    }

    inline std::string as_string(region range)
    {
        return std::string(range.start.as<const char*>(), range.size);
    }

    inline std::string as_hex(region range, bool upper_case, bool padded)
    {
        const char* const char_hex_table = upper_case ? "0123456789ABCDEF" : "0123456789abcdef";

        std::string result;

        result.reserve(range.size * (padded ? 3 : 2));

        for (std::size_t i = 0; i < range.size; ++i)
        {
            if (i && padded)
            {
                result.push_back(' ');
            }

            const byte value = range.start.at<const byte>(i);

            result.push_back(char_hex_table[(value >> 4) & 0xF]);
            result.push_back(char_hex_table[(value >> 0) & 0xF]);
        }

        return result;
    }

    inline std::vector<byte> unescape(const char* string, std::size_t length, bool strict)
    {
        std::vector<byte> results;

        char_queue input(string, length);

        while (input)
        {
            std::size_t result = SIZE_MAX;
            std::size_t count = 0;

            bool output_utf8 = false;

            int current = -1;
            int temp = -1;

            current = input.peek();
            input.pop();

            if (current == '\\')
            { // clang-format off
                current = input.peek();
                input.pop();

                if      (current == '\'') { result = 0x27; }
                else if (current == '\"') { result = 0x22; }
                else if (current == '\\') { result = 0x5C; }
                else if (current == '?')  { result = 0x3F; }
                else if (current == 'a')  { result = 0x07; }
                else if (current == 'b')  { result = 0x08; }
                else if (current == 'e')  { result = 0x1B; }
                else if (current == 'f')  { result = 0x0C; }
                else if (current == 'n')  { result = 0x0A; }
                else if (current == 'r')  { result = 0x0D; }
                else if (current == 't')  { result = 0x09; }
                else if (current == 'v')  { result = 0x0B; }
                else if (current == 'x')
                {
                    result = 0;
                    count = 0;

                    while ((temp = xctoi(input.peek())) != -1)
                    {
                        input.pop();
                        result = (result * 16) + static_cast<std::size_t>(temp);
                        ++count;
                    }

                    if (strict && (count == 0))
                    {
                        return {};
                    }
                }
                else if (current == 'u' || current == 'U')
                {
                    result = 0;
                    count = 0;
                    output_utf8 = true;

                    std::size_t limit = (current == 'U') ? 8 : 4;

                    while ((temp = xctoi(input.peek())) != -1)
                    {
                        input.pop();
                        result = (result * 16) + static_cast<std::size_t>(temp);
                        ++count;

                        if (count == limit)
                        {
                            break;
                        }
                    }

                    if (strict && (count != limit))
                    {
                        return {};
                    }
                }
                else if ((temp = octoi(current)) != -1)
                {
                    result = static_cast<std::size_t>(temp);
                    count = 1;

                    while ((temp = octoi(input.peek())) != -1)
                    {
                        input.pop();
                        result = (result * 8) + static_cast<std::size_t>(temp);
                        ++count;

                        if (count == 3)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    if (strict)
                    {
                        return {};
                    }

                    result = static_cast<std::size_t>(current);
                }
            } // clang-format on
            else
            {
                result = static_cast<std::size_t>(current);
            }

            if (output_utf8)
            {
                if ((result > 0x10FFFF) || ((result >= 0xD800) && (result <= 0xDFFF)))
                {
                    if (strict)
                    {
                        return {};
                    }
                    else
                    {
                        result = 0xFFFD;
                    }
                }

                byte values[4];
                byte* values_end = values;

                if (result < 0x80)
                {
                    *values_end++ = static_cast<byte>(result);
                }
                else
                {
                    if (result < 0x800)
                    {
                        *values_end++ = static_cast<byte>((result >> 6) | 0xC0);
                    }
                    else
                    {
                        if (result < 0x10000)
                        {
                            *values_end++ = static_cast<byte>((result >> 12) | 0xE0);
                        }
                        else
                        {
                            *values_end++ = static_cast<byte>((result >> 18) | 0xF0);
                            *values_end++ = static_cast<byte>(((result >> 12) & 0x3F) | 0x80);
                        }

                        *values_end++ = static_cast<byte>(((result >> 6) & 0x3F) | 0x80);
                    }

                    *values_end++ = static_cast<byte>((result & 0x3F) | 0x80);
                }

                results.insert(results.end(), values, values_end);
            }
            else
            {
                if (result > UCHAR_MAX)
                {
                    if (strict)
                    {
                        return {};
                    }
                    else
                    {
                        result &= UCHAR_MAX;
                    }
                }

                results.push_back(static_cast<byte>(result));
            }
        }

        return results;
    }
} // namespace mem

#endif // MEM_UTILS_BRICK_H
