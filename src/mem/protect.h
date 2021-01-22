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

#ifndef MEM_PROTECT_BRICK_H
#define MEM_PROTECT_BRICK_H

#include "mem.h"
#include "prot_flags.h"

#include <cstdio>

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#elif defined(__unix__)
#    if !defined(_GNU_SOURCE)
#        define _GNU_SOURCE
#    endif
#    include <cinttypes>
#    include <sys/mman.h>
#    include <unistd.h>
#else
#    error Unknown Platform
#endif

namespace mem
{
    std::size_t page_size();

    void* protect_alloc(std::size_t length, prot_flags flags);
    void protect_free(void* memory, std::size_t length);

    prot_flags protect_query(void* memory);

    bool protect_modify(void* memory, std::size_t length, prot_flags flags, prot_flags* old_flags = nullptr);

#if defined(__unix__)
    struct region_info
    {
        std::uintptr_t start;
        std::uintptr_t end;
        std::size_t offset;
        int prot;
        int flags;
        const char* path_name;
    };

    int iter_proc_maps(int (*callback)(region_info*, void*), void* data);
#endif

    class protect : public region
    {
    private:
        prot_flags old_flags_ {prot_flags::INVALID};
        bool success_ {false};

    public:
        protect(region range, prot_flags flags = prot_flags::RWX);
        ~protect();

        protect(protect&& rhs) noexcept;
        protect(const protect&) = delete;

        explicit operator bool() const noexcept;
        prot_flags release() noexcept;
    };

    inline std::size_t page_size()
    {
#if defined(_WIN32)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return static_cast<std::size_t>(si.dwPageSize);
#elif defined(__unix__)
        return static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#endif
    }

    inline void* protect_alloc(std::size_t length, prot_flags flags)
    {
#if defined(_WIN32)
        return VirtualAlloc(nullptr, length, MEM_RESERVE | MEM_COMMIT, from_prot_flags(flags));
#elif defined(__unix__)
        void* result = mmap(nullptr, length, from_prot_flags(flags), MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (result == MAP_FAILED)
            result = nullptr;
        return result;
#endif
    }

    inline void protect_free(void* memory, std::size_t length)
    {
        if (memory != nullptr)
        {
#if defined(_WIN32)
            (void) length;
            VirtualFree(memory, 0, MEM_RELEASE);
#elif defined(__unix__)
            munmap(memory, length);
#endif
        }
    }

#if defined(__unix__)
    namespace internal
    {
        struct prot_query
        {
            std::uintptr_t address;
            prot_flags result;
        };

        inline int prot_query_callback(region_info* region, void* data)
        {
            prot_query* query = static_cast<prot_query*>(data);

            if ((query->address >= region->start) && (query->address < region->end))
            {
                query->result = to_prot_flags(region->prot);

                return 1;
            }

            return 0;
        }
    } // namespace internal
#endif

    inline prot_flags protect_query(void* memory)
    {
#if defined(_WIN32)
        MEMORY_BASIC_INFORMATION info;

        if (VirtualQuery(memory, &info, sizeof(info)))
            return to_prot_flags(info.Protect);

        return prot_flags::INVALID;
#elif defined(__unix__)
        internal::prot_query query;
        query.address = reinterpret_cast<std::uintptr_t>(memory);

        if (iter_proc_maps(&internal::prot_query_callback, &query))
        {
            return query.result;
        }

        return prot_flags::INVALID;
#endif
    }

    inline bool protect_modify(void* memory, std::size_t length, prot_flags flags, prot_flags* old_flags)
    {
        if (flags == prot_flags::INVALID)
            return false;

#if defined(_WIN32)
        DWORD old_protect = 0;
        bool success = VirtualProtect(memory, length, from_prot_flags(flags), &old_protect) != FALSE;

        if (old_flags)
            *old_flags = success ? to_prot_flags(old_protect) : prot_flags::INVALID;

        return success;
#elif defined(__unix__)
        if (old_flags)
            *old_flags = protect_query(memory);

        return mprotect(memory, length, from_prot_flags(flags)) == 0;
#endif
    }

#if defined(__unix__)
    inline int iter_proc_maps(int (*callback)(region_info*, void*), void* data)
    {
        std::FILE* maps = std::fopen("/proc/self/maps", "r");

        int result = 0;

        if (maps != nullptr)
        {
            char buffer[256];

            region_info region;

            char perms[5];
            char pathname[256];

            while (std::fgets(buffer, 256, maps))
            {
                int count = std::sscanf(buffer, "%" SCNxPTR "-%" SCNxPTR " %4s %zx %*x:%*x %*u %255s", &region.start,
                    &region.end, perms, &region.offset, pathname);

                if (count < 4)
                    continue;

                region.prot = PROT_NONE;
                region.flags = 0;

                if (perms[0] == 'r')
                    region.prot |= PROT_READ;

                if (perms[1] == 'w')
                    region.prot |= PROT_WRITE;

                if (perms[2] == 'x')
                    region.prot |= PROT_EXEC;

                if (perms[3] == 's')
                    region.flags |= MAP_SHARED;
                else if (perms[3] == 'p')
                    region.flags |= MAP_PRIVATE;

                if (count > 4)
                {
                    region.path_name = pathname;
                }
                else
                {
                    region.flags |= MAP_ANONYMOUS;
                    region.path_name = nullptr;
                }

                result = callback(&region, data);

                if (result)
                    break;
            }

            std::fclose(maps);
        }

        return result;
    }
#endif

    MEM_STRONG_INLINE protect::protect(region range, prot_flags flags)
        : region(range)
        , old_flags_(prot_flags::INVALID)
        , success_(protect_modify(start.as<void*>(), size, flags, &old_flags_))
    {}

    MEM_STRONG_INLINE protect::~protect()
    {
        if (success_)
        {
            protect_modify(start.as<void*>(), size, old_flags_, nullptr);
        }
    }

    MEM_STRONG_INLINE protect::protect(protect&& rhs) noexcept
        : region(rhs)
        , old_flags_(rhs.old_flags_)
        , success_(rhs.success_)
    {
        rhs.old_flags_ = prot_flags::INVALID;
        rhs.success_ = false;
    }

    MEM_STRONG_INLINE protect::operator bool() const noexcept
    {
        return success_;
    }

    MEM_STRONG_INLINE prot_flags protect::release() noexcept
    {
        success_ = false;

        return old_flags_;
    }
} // namespace mem

#endif // MEM_PROTECT_BRICK_H
