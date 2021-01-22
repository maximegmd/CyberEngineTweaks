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

#ifndef MEM_PROT_FLAGS_BRICK_H
#define MEM_PROT_FLAGS_BRICK_H

#include "bitwise_enum.h"
#include "defines.h"

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#elif defined(__unix__)
#    include <sys/mman.h>
#else
#    error Unknown Platform
#endif

namespace mem
{
    namespace enums
    {
        enum prot_flags : std::uint32_t
        { // clang-format off
            NONE = 0x0, // No Access

            R = 0x1, // Read
            W = 0x2, // Write
            X = 0x4, // Execute
            C = 0x8, // Copy On Write

            G = 0x10, // Guard

            INVALID = 0x80000000, // Invalid

            RW   = R | W,
            RX   = R     | X,
            RWX  = R | W | X,
            RWC  = R | W     | C,
            RWXC = R | W | X | C,
        }; // clang-format on

        MEM_DEFINE_ENUM_FLAG_OPERATORS(prot_flags)
    } // namespace enums

    using enums::prot_flags;

#if defined(_WIN32)
    using native_prot = DWORD;
#elif defined(__unix__)
    using native_prot = int;
#endif

    MEM_CONSTEXPR_14 native_prot from_prot_flags(prot_flags flags) noexcept;
    MEM_CONSTEXPR_14 prot_flags to_prot_flags(native_prot flags) noexcept;

    inline MEM_CONSTEXPR_14 native_prot from_prot_flags(prot_flags flags) noexcept
    { // clang-format off
#if defined(_WIN32)
        native_prot result = PAGE_NOACCESS;

        if (flags & prot_flags::X)
        {
            if      (flags & prot_flags::C) { result = PAGE_EXECUTE_WRITECOPY; }
            else if (flags & prot_flags::W) { result = PAGE_EXECUTE_READWRITE; }
            else if (flags & prot_flags::R) { result = PAGE_EXECUTE_READ;      }
            else                            { result = PAGE_EXECUTE;           }
        }
        else
        {
            if      (flags & prot_flags::C) { result = PAGE_EXECUTE_WRITECOPY; }
            else if (flags & prot_flags::W) { result = PAGE_READWRITE; }
            else if (flags & prot_flags::R) { result = PAGE_READONLY;  }
            else                            { result = PAGE_NOACCESS;  }
        }

        if (flags & prot_flags::G)
            result |= PAGE_GUARD;

        return result;
#elif defined(__unix__)
        native_prot result = 0;

        if (flags & prot_flags::R)
            result |= PROT_READ;
        if (flags & prot_flags::W)
            result |= PROT_WRITE;
        if (flags & prot_flags::X)
            result |= PROT_EXEC;

        return result;
#endif
    } // clang-format on

    inline MEM_CONSTEXPR_14 prot_flags to_prot_flags(native_prot flags) noexcept
    { // clang-format off
#if defined(_WIN32)
        prot_flags result = prot_flags::NONE;

        switch (flags & 0xFF)
        {
            case PAGE_EXECUTE:
                result = prot_flags::X;
                break;
            case PAGE_EXECUTE_READ:
                result = prot_flags::RX;
                break;
            case PAGE_EXECUTE_READWRITE:
                result = prot_flags::RWX;
                break;
            case PAGE_EXECUTE_WRITECOPY:
                result = prot_flags::RWXC;
                break;
            case PAGE_NOACCESS:
                result = prot_flags::NONE;
                break;
            case PAGE_READONLY:
                result = prot_flags::R;
                break;
            case PAGE_READWRITE:
                result = prot_flags::RW;
                break;
            case PAGE_WRITECOPY:
                result = prot_flags::RWC;
                break;
            default:
                result = prot_flags::INVALID;
                break;
        }

        if (flags & PAGE_GUARD)
            result |= prot_flags::G;

        return result;
#elif defined(__unix__)
        prot_flags result = prot_flags::NONE;

        if (flags & PROT_READ)
            result |= prot_flags::R;

        if (flags & PROT_WRITE)
            result |= prot_flags::W;

        if (flags & PROT_EXEC)
            result |= prot_flags::X;

        return result;
#endif
    } // clang-format on
} // namespace mem

#endif // MEM_PROT_FLAGS_BRICK_H
