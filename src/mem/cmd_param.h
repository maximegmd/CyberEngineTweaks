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

#ifndef MEM_CMD_PARAM_BRICK_H
#define MEM_CMD_PARAM_BRICK_H

#include "defines.h"

#include <cstdlib>

#include <istream>
#include <streambuf>

namespace mem
{
    class cmd_param
    {
    public:
        cmd_param(const char* name, int pos = 0) noexcept;
        ~cmd_param();

        cmd_param(const cmd_param&) = delete;
        cmd_param(cmd_param&&) = delete;

        template <typename T = const char*>
        T get() const;

        template <typename T>
        bool get(T& out) const;

        template <typename T>
        T get_or(T value) const;

        explicit operator bool() const;

        static void init(const char* const* argv);
        static void init(int argc, const char* const* argv);
        static void reset();

    private:
        const char* name_ {nullptr};
        int pos_ {0};
        const char* value_ {nullptr};
        cmd_param* next_ {nullptr};

        static cmd_param* ROOT;
    };

    MEM_STRONG_INLINE cmd_param::cmd_param(const char* name, int pos) noexcept
        : name_(name)
        , pos_(pos)
        , next_(ROOT)
    {
        ROOT = this;
    }

    inline cmd_param::~cmd_param()
    {
        for (cmd_param** i = &ROOT; *i; i = &(*i)->next_)
        {
            if (*i == this)
            {
                *i = next_;

                break;
            }
        }
    }

    namespace internal
    {
        template <typename T>
        class fixed_basic_streambuf final : public std::basic_streambuf<T>
        {
        public:
            fixed_basic_streambuf(const T* ptr, std::size_t num)
            {
                T* source = const_cast<T*>(ptr);

                std::basic_streambuf<T>::setg(source, source, source + num);
            }
        };

        using fixed_streambuf = fixed_basic_streambuf<char>;
    } // namespace internal

    template <typename T>
    MEM_STRONG_INLINE bool parse_cmd(const char* value, T& out)
    {
        internal::fixed_streambuf buf(value, std::strlen(value));

        std::istream strm(&buf);

        strm >> out;

        return strm.good();
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<bool>(const char* value, bool& out)
    {
        if (!std::strcmp(value, "false") || !std::strcmp(value, "0"))
        {
            out = false;
        }
        else
        {
            out = true;
        }

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<short>(const char* value, short& out)
    {
        out = static_cast<short>(std::strtol(value, nullptr, 0));

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<unsigned short>(const char* value, unsigned short& out)
    {
        out = static_cast<unsigned short>(std::strtoul(value, nullptr, 0));

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<int>(const char* value, int& out)
    {
        out = static_cast<int>(std::strtol(value, nullptr, 0));

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<unsigned int>(const char* value, unsigned int& out)
    {
        out = static_cast<unsigned int>(std::strtoul(value, nullptr, 0));

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<long>(const char* value, long& out)
    {
        out = std::strtol(value, nullptr, 0);

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<unsigned long>(const char* value, unsigned long& out)
    {
        out = std::strtoul(value, nullptr, 0);

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<long long>(const char* value, long long& out)
    {
        out = std::strtoll(value, nullptr, 0);

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<unsigned long long>(const char* value, unsigned long long& out)
    {
        out = std::strtoull(value, nullptr, 0);

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<float>(const char* value, float& out)
    {
        out = std::strtof(value, nullptr);

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<double>(const char* value, double& out)
    {
        out = std::strtod(value, nullptr);

        return true;
    }

    template <>
    MEM_STRONG_INLINE bool parse_cmd<long double>(const char* value, long double& out)
    {
        out = std::strtold(value, nullptr);

        return true;
    }

    template <>
    MEM_STRONG_INLINE const char* cmd_param::get<const char*>() const
    {
        return value_;
    }

    template <>
    MEM_STRONG_INLINE bool cmd_param::get<bool>() const
    {
        bool result = false;

        if (value_)
        {
            parse_cmd(value_, result);
        }

        return result;
    }

    template <typename T>
    MEM_STRONG_INLINE bool cmd_param::get(T& out) const
    {
        if (value_)
        {
            return parse_cmd(value_, out);
        }

        return false;
    }

    template <typename T>
    MEM_STRONG_INLINE T cmd_param::get_or(T value) const
    {
        if (value_)
        {
            parse_cmd(value_, value);
        }

        return value;
    }

    MEM_STRONG_INLINE cmd_param::operator bool() const
    {
        return get<bool>();
    }
} // namespace mem

#endif // MEM_CMD_PARAM_BRICK_H
