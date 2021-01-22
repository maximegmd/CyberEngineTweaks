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

#ifndef MEM_MACROS_BRICK_H
#define MEM_MACROS_BRICK_H

#include "init_function.h"
#include "mem.h"

#define mem_decl_extern(TYPE, NAME) typename std::add_lvalue_reference<TYPE>::type NAME
#define mem_defn_extern(ADDRESS, NAME) decltype(NAME) NAME = mem::pointer(ADDRESS).as<decltype(NAME)>()

#ifdef __INTELLISENSE__
#    define mem_extern_var(ADDRESS, TYPE, NAME)               \
        typename std::add_lvalue_reference<TYPE>::type NAME = \
            *static_cast<typename std::add_pointer<TYPE>::type>(nullptr)
#else
#    define mem_extern_var(ADDRESS, TYPE, NAME)               \
        typename std::add_lvalue_reference<TYPE>::type NAME = \
            mem::pointer(ADDRESS).as<typename std::add_lvalue_reference<TYPE>::type>()
#endif

#define mem_check_size(TYPE, SIZE)                                                         \
    static_assert(sizeof(TYPE) >= (SIZE), #TYPE " too small: sizeof(" #TYPE ") < " #SIZE); \
    static_assert(sizeof(TYPE) <= (SIZE), #TYPE " too large: sizeof(" #TYPE ") > " #SIZE)

#define mem_paste_(LHS, RHS) LHS##RHS
#define mem_paste(LHS, RHS) mem_paste_(LHS, RHS)

#define mem_str_(VALUE) #VALUE
#define mem_str(VALUE) mem_str_(VALUE)

#define mem_run_once static mem::init_function mem_paste(run_once_, __LINE__)

#if defined(_MSC_VER)
#    define mem_define_dummy_symbol(NAME)         \
        extern "C" namespace dummy                \
        {                                         \
            void mem_paste(dummy_symbol_, NAME)() \
            {}                                    \
        }
#    if defined(MEM_ARCH_X86)
#        define dummy_symbol_prefix "_"
#    else
#        define dummy_symbol_prefix ""
#    endif
#    define mem_include_dummy_symbol(NAME) \
        __pragma(comment(linker, "/INCLUDE:" dummy_symbol_prefix mem_str(mem_paste(dummy_symbol_, NAME))))
#endif

#endif // MEM_MACROS_BRICK_H
