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

#if defined(MEM_CMD_PARAM_INL_BRICK_H)
#    error mem/cmd_param-inl.h should only be included once
#endif // MEM_CMD_PARAM_INL_BRICK_H

#define MEM_CMD_PARAM_INL_BRICK_H

#if !defined(CMD_ALLOC_BUFFER_CAPACITY)
#    define CMD_ALLOC_BUFFER_CAPACITY 0x1000
#endif

#include "cmd_param.h"

#include <cctype>
#include <cstring>

namespace mem
{
    cmd_param* cmd_param::ROOT {nullptr};

    static inline bool cmd_is_option(const char* arg)
    {
        return (arg[0] == '-') && (static_cast<unsigned char>(arg[1] - '0') > 9);
    }

    static inline bool cmd_chr_equal(char lhs, char rhs)
    {
        const char x = rhs ^ lhs;

        return (x == 0) || ((x == 0x20) && (static_cast<unsigned char>((lhs | 0x20) - 'a') < 26));
    }

    static char cmd_alloc_buffer[CMD_ALLOC_BUFFER_CAPACITY];
    static std::size_t cmd_alloc_buffer_len {0};

    static char* cmd_strdup(const char* value, std::size_t len)
    {
        if (cmd_alloc_buffer_len + len >= CMD_ALLOC_BUFFER_CAPACITY)
            return nullptr;

        char* result = &cmd_alloc_buffer[cmd_alloc_buffer_len];
        std::memcpy(result, value, len);
        result[len] = '\0';

        cmd_alloc_buffer_len += len + 1;

        return result;
    }

    static char* cmd_unquote(const char* arg)
    {
        std::size_t len = std::strlen(arg);

        if (arg[0] == '"')
        {
            ++arg;
            --len;

            if (arg[len - 1] == '"')
                --len;
        }

        return cmd_strdup(arg, len);
    }

    static bool cmd_arg_equal(const char* lhs, const char* rhs)
    {
        while (true)
        {
            char a = *lhs++;
            char b = *rhs++;

            if (a == '\0')
                return (b == '\0') || (b == '=');

            if (!cmd_chr_equal(a, b))
                return false;
        }
    }

    void cmd_param::init(const char* const* argv)
    {
        int argc = 0;

        while (argv[argc])
            ++argc;

        init(argc, argv);
    }

    void cmd_param::init(int argc, const char* const* argv)
    {
        bool done_positionals = false;

        for (int i = 1; i < argc; ++i)
        {
            const char* arg = argv[i];

            if (cmd_is_option(arg))
            {
                while (arg[0] == '-')
                    ++arg;

                done_positionals = true;

                const char* value = nullptr;

                for (cmd_param* j = ROOT; j; j = j->next_)
                {
                    if (!j->name_ || !cmd_arg_equal(j->name_, arg))
                        continue;

                    if (!value)
                    {
                        if (const char* val = std::strchr(arg, '='))
                            value = cmd_unquote(val + 1);
                        else if (i + 1 < argc && !cmd_is_option(argv[i + 1]))
                            value = cmd_unquote(argv[i + 1]);

                        if (!value)
                            value = "";
                    }

                    j->value_ = value;
                }

                if (!value)
                {
                    for (cmd_param* j = ROOT; j; j = j->next_)
                    {
                        if (!j->name_)
                            continue;

                        // clang-format off
                        if ((!std::strncmp("no", arg,      2) && cmd_arg_equal(j->name_, arg + 2)) ||
                            (!std::strncmp("no", j->name_, 2) && cmd_arg_equal(j->name_ + 2, arg)))
                            j->value_ = "false";
                        // clang-format on
                    }
                }
            }
            else if (!done_positionals)
            {
                const char* value = nullptr;

                for (cmd_param* j = ROOT; j; j = j->next_)
                {
                    if (j->pos_ != i)
                        continue;

                    if (!value)
                    {
                        value = cmd_unquote(arg);

                        if (!value)
                            value = "";
                    }

                    j->value_ = value;
                }
            }
        }
    }

    void cmd_param::reset()
    {
        for (cmd_param* j = ROOT; j; j = j->next_)
            j->value_ = nullptr;

        cmd_alloc_buffer_len = 0;
    }
} // namespace mem
