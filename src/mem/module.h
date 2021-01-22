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

#ifndef MEM_MODULE_BRICK_H
#define MEM_MODULE_BRICK_H

#include "mem.h"
#include "prot_flags.h"
#include "slice.h"

#include <cstdio>
#include <cstdlib>
#include <memory>

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#    include <intrin.h>
#    if defined(_WIN64)
#        pragma intrinsic(__readgsqword)
#    else
#        pragma intrinsic(__readfsdword)
#    endif
#elif defined(__unix__)
#    ifndef _GNU_SOURCE
#        define _GNU_SOURCE
#    endif
#    include <link.h>
#    if defined(MEM_USE_DLFCN) // Requires -ldl linker flag
#        include <dlfcn.h>
#    endif
#else
#    error Unknown Platform
#endif

namespace mem
{
    class module : public region
    {
    public:
        using region::region;

#if defined(_WIN32)
        static module nt(pointer address);
        static module named(const wchar_t* name);

        const IMAGE_DOS_HEADER& dos_header();
        const IMAGE_NT_HEADERS& nt_headers();
        slice<const IMAGE_SECTION_HEADER> section_headers();
#elif defined(__unix__)
        static module elf(pointer address);

        const ElfW(Ehdr) & elf_header();
        slice<const ElfW(Phdr)> program_headers();
        slice<const ElfW(Shdr)> section_headers();
#endif

        static module named(const char* name);

        static module main();
        static module self();

        template <typename Func>
        void enum_segments(Func func);

#if defined(_WIN32)
        template <typename Func>
        void enum_exports(Func func);
#endif
    };

#if defined(_WIN32)
    namespace internal
    {
        extern "C" IMAGE_DOS_HEADER __ImageBase;
    }

    struct PEB
    {
        UCHAR InheritedAddressSpace;
        UCHAR ReadImageFileExecOptions;
        UCHAR BeingDebugged;
        UCHAR Spare;
        PVOID Mutant;
        PVOID ImageBaseAddress;
    };

    MEM_STRONG_INLINE PEB* get_peb() noexcept
    {
#    if defined(_WIN64)
        return reinterpret_cast<PEB*>(__readgsqword(0x60));
#    else
        return reinterpret_cast<PEB*>(__readfsdword(0x30));
#    endif
    }

    MEM_STRONG_INLINE module module::nt(pointer address)
    {
        if (!address)
            return module();

        const IMAGE_DOS_HEADER& dos = address.at<const IMAGE_DOS_HEADER>(0);

        if (dos.e_magic != IMAGE_DOS_SIGNATURE)
            return module();

        const IMAGE_NT_HEADERS& nt = address.at<const IMAGE_NT_HEADERS>(dos.e_lfanew);

        if (nt.Signature != IMAGE_NT_SIGNATURE)
            return module();

        if (nt.FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER))
            return module();

        return module(address, nt.OptionalHeader.SizeOfImage);
    }

    MEM_STRONG_INLINE module module::named(const char* name)
    {
        return nt(GetModuleHandleA(name));
    }

    MEM_STRONG_INLINE module module::named(const wchar_t* name)
    {
        return nt(GetModuleHandleW(name));
    }

    MEM_STRONG_INLINE module module::main()
    {
        return nt(get_peb()->ImageBaseAddress);
    }

    MEM_STRONG_INLINE module module::self()
    {
        return nt(&internal::__ImageBase);
    }

    MEM_STRONG_INLINE const IMAGE_DOS_HEADER& module::dos_header()
    {
        return start.at<const IMAGE_DOS_HEADER>(0);
    }

    MEM_STRONG_INLINE const IMAGE_NT_HEADERS& module::nt_headers()
    {
        return start.at<const IMAGE_NT_HEADERS>(dos_header().e_lfanew);
    }

    MEM_STRONG_INLINE slice<const IMAGE_SECTION_HEADER> module::section_headers()
    {
        const IMAGE_NT_HEADERS& nt = nt_headers();
        const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(&nt);

        return {sections, nt.FileHeader.NumberOfSections};
    }

    template <typename Func>
    MEM_STRONG_INLINE void module::enum_segments(Func func)
    {
        for (const IMAGE_SECTION_HEADER& section : section_headers())
        {
            mem::region range(start.add(section.VirtualAddress), section.Misc.VirtualSize);

            if (!range.size)
                continue;

            prot_flags prot = prot_flags::NONE;

            if (section.Characteristics & IMAGE_SCN_MEM_READ)
                prot |= prot_flags::R;

            if (section.Characteristics & IMAGE_SCN_MEM_WRITE)
                prot |= prot_flags::W;

            if (section.Characteristics & IMAGE_SCN_MEM_EXECUTE)
                prot |= prot_flags::X;

            if (func(range, prot))
                return;
        }
    }

    template <typename Func>
    MEM_STRONG_INLINE void module::enum_exports(Func func)
    {
        const IMAGE_DATA_DIRECTORY& export_data_dir =
            nt_headers().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

        if (export_data_dir.Size < sizeof(IMAGE_EXPORT_DIRECTORY))
            return;

        const IMAGE_EXPORT_DIRECTORY& export_dir =
            start.add(export_data_dir.VirtualAddress).as<const IMAGE_EXPORT_DIRECTORY&>();

        const uint32_t name_count = export_dir.NumberOfNames;
        const uint32_t func_count = export_dir.NumberOfFunctions;

        const uint32_t* const names = start.add(export_dir.AddressOfNames).as<const uint32_t*>();
        const uint16_t* const ordinals = start.add(export_dir.AddressOfNameOrdinals).as<const uint16_t*>();
        const uint32_t* const functions = start.add(export_dir.AddressOfFunctions).as<const uint32_t*>();

        for (uint32_t i = 0; i < func_count; ++i)
        {
            const char* name = (i < name_count) ? start.add(names[i]).as<const char*>() : nullptr;
            const uint16_t ordinal = ordinals[i];
            const pointer function = start.add(functions[ordinal]);

            if (func(name, ordinal, function))
                break;
        }
    }

#elif defined(__unix__)
    // https://github.com/torvalds/linux/blob/master/fs/binfmt_elf.c
    inline std::size_t total_mapping_size(const ElfW(Phdr) * cmds, std::size_t count)
    {
        std::size_t first_idx = SIZE_MAX, last_idx = SIZE_MAX;

        for (std::size_t i = 0; i < count; ++i)
        {
            if (cmds[i].p_type == PT_LOAD)
            {
                last_idx = i;

                if (first_idx == SIZE_MAX)
                    first_idx = i;
            }
        }

        if (first_idx == SIZE_MAX)
            return 0;

        return cmds[last_idx].p_vaddr + cmds[last_idx].p_memsz -
            (cmds[first_idx].p_vaddr & ~(cmds[first_idx].p_align - 1));
    }

    MEM_STRONG_INLINE module module::elf(pointer address)
    {
        if (!address)
            return module();

        const ElfW(Ehdr)& ehdr = address.at<const ElfW(Ehdr)&>(0);

        // clang-format off
        if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
            ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
            ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
            ehdr.e_ident[EI_MAG3] != ELFMAG3)
            return module();

        if (ehdr.e_phentsize != sizeof(ElfW(Phdr)) ||
            ehdr.e_shentsize != sizeof(ElfW(Shdr)))
            return module();
        // clang-format on

        const ElfW(Phdr)* phdr = address.at<const ElfW(Phdr)[]>(ehdr.e_phoff);
        const std::size_t mapping_size = total_mapping_size(phdr, ehdr.e_phnum);

        return module(address, mapping_size);
    }

    MEM_STRONG_INLINE const ElfW(Ehdr) & module::elf_header()
    {
        return start.at<const ElfW(Ehdr)>(0);
    }

    MEM_STRONG_INLINE slice<const ElfW(Phdr)> module::program_headers()
    {
        const ElfW(Ehdr)& ehdr = elf_header();
        const ElfW(Phdr)* phdr = start.at<const ElfW(Phdr)[]>(ehdr.e_phoff);

        return {phdr, ehdr.e_phnum};
    }

    MEM_STRONG_INLINE slice<const ElfW(Shdr)> module::section_headers()
    {
        const ElfW(Ehdr)& ehdr = elf_header();
        const ElfW(Shdr)* shdr = start.at<const ElfW(Shdr)[]>(ehdr.e_shoff);

        return {shdr, ehdr.e_shnum};
    }

    template <typename Func>
    MEM_STRONG_INLINE void module::enum_segments(Func func)
    {
        for (const ElfW(Phdr) & section : program_headers())
        {
            if (section.p_type != PT_LOAD)
                continue;

            if (!section.p_memsz)
                continue;

            mem::region range(start.add(section.p_vaddr), section.p_memsz);

            prot_flags prot = prot_flags::NONE;

            if (section.p_flags & PF_R)
                prot |= prot_flags::R;

            if (section.p_flags & PF_W)
                prot |= prot_flags::W;

            if (section.p_flags & PF_X)
                prot |= prot_flags::X;

            if (func(range, prot))
                return;
        }
    }

    MEM_STRONG_INLINE module module::main()
    {
        return named(nullptr);
    }

    namespace internal
    {
        extern "C" ElfW(Ehdr) __ehdr_start;
    }

    MEM_STRONG_INLINE module module::self()
    {
        return elf(&internal::__ehdr_start);
    }

#    if defined(MEM_USE_DLFCN)
    MEM_STRONG_INLINE module module::named(const char* name)
    {
        void* handle = dlopen(name, RTLD_LAZY | RTLD_NOLOAD);

        if (handle)
        {
#        if defined(MEM_USE_DLINFO)
            const link_map* lm = nullptr;

            if (dlinfo(handle, RTLD_DI_LINKMAP, &lm))
                lm = nullptr;
#        else
            const link_map* lm = static_cast<const link_map*>(handle);
#        endif

            void* base_addr = nullptr;

            if (lm && lm->l_ld)
            {
                Dl_info info;

                if (dladdr(lm->l_ld, &info))
                {
                    base_addr = info.dli_fbase;
                }
            }

            dlclose(handle);

            return elf(base_addr);
        }

        return module();
    }
#    else
    namespace internal
    {
        struct dl_iterate_query
        {
            const char* name {nullptr};
            void* result {nullptr};
        };

        inline int dl_iterate_callback(struct dl_phdr_info* info, std::size_t size, void* data)
        {
            (void) size;

            dl_iterate_query* search_info = static_cast<dl_iterate_query*>(data);

            const char* file_path = info->dlpi_name;
            const char* file_name = std::strrchr(file_path, '/');

            if (file_name)
            {
                ++file_name;
            }
            else
            {
                file_name = file_path;
            }

            if (!std::strcmp(search_info->name, file_name))
            {
                for (int i = 0; i < info->dlpi_phnum; ++i)
                {
                    if (info->dlpi_phdr[i].p_type == PT_LOAD)
                    {
                        search_info->result = reinterpret_cast<void*>(info->dlpi_addr + info->dlpi_phdr[i].p_vaddr);

                        return 1;
                    }
                }

                return 2;
            }

            return 0;
        }
    } // namespace internal

    MEM_STRONG_INLINE module module::named(const char* name)
    {
        internal::dl_iterate_query search;

        search.name = name ? name : "";

        if (dl_iterate_phdr(&internal::dl_iterate_callback, &search) == 1)
        {
            return elf(search.result);
        }

        return module();
    }
#    endif
#endif
} // namespace mem

#endif // MEM_MODULE_BRICK_H
