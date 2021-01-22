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

#ifndef MEM_EXECUTION_HANDLER_BRICK_H
#define MEM_EXECUTION_HANDLER_BRICK_H

#include "defines.h"

#include <memory>
#include <stdexcept>

#if defined(_WIN32)
#    if !defined(WIN32_LEAN_AND_MEAN)
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <Windows.h>
#    include <eh.h>
#elif defined(__unix__)
#    include <setjmp.h>
#    include <signal.h>
#else
#    error Unknown Platform
#endif

namespace mem
{
#if defined(_WIN32)
    class scoped_seh
    {
    private:
        _se_translator_function old_handler_ {nullptr};

    public:
        scoped_seh();
        ~scoped_seh();

        scoped_seh(const scoped_seh&) = delete;
        scoped_seh(scoped_seh&&) = delete;

        template <typename Func, typename... Args>
        auto execute(Func func, Args&&... args) -> decltype(func(std::forward<Args>(args)...))
        {
            return func(std::forward<Args>(args)...);
        }
    };

    using execution_handler = scoped_seh;
#elif defined(__unix__)
    class signal_handler
    {
    private:
        std::unique_ptr<char[]> sig_stack_;
        stack_t old_stack_;
        struct sigaction old_actions_[3];
        sigjmp_buf jmp_buffer_;

        static void sig_handler(int sig, siginfo_t* info, void* ucontext);
        static signal_handler*& current_handler();

        class scoped_handler
        {
        private:
            signal_handler* prev_ {nullptr};

        public:
            scoped_handler(signal_handler* handler);
            ~scoped_handler();

            scoped_handler(const scoped_handler&) = delete;
            scoped_handler(scoped_handler&&) = delete;
        };

    public:
        signal_handler();
        ~signal_handler();

        signal_handler(const signal_handler&) = delete;
        signal_handler(signal_handler&&) = delete;

        template <typename Func, typename... Args>
        auto execute(Func func, Args&&... args) -> decltype(func(std::forward<Args>(args)...))
        {
            scoped_handler scope(this);

            if (sigsetjmp(jmp_buffer_, 1))
            {
                throw std::runtime_error("Execution Error");
            }

            return func(std::forward<Args>(args)...);
        }
    };

    using execution_handler = signal_handler;
#endif

#if defined(_WIN32)
    namespace internal
    {
        inline MEM_CONSTEXPR_14 const char* translate_exception_code(std::uint32_t code) noexcept
        {
            switch (code)
            {
                case 0x80000001: return "STATUS_GUARD_PAGE_VIOLATION";
                case 0x80000002: return "STATUS_DATATYPE_MISALIGNMENT";
                case 0x80000003: return "STATUS_BREAKPOINT";
                case 0x80000004: return "STATUS_SINGLE_STEP";
                case 0x80000026: return "STATUS_LONGJUMP";
                case 0x80000029: return "STATUS_UNWIND_CONSOLIDATE";
                case 0x80010001: return "DBG_EXCEPTION_NOT_HANDLED";
                case 0xC0000005: return "STATUS_ACCESS_VIOLATION";
                case 0xC0000006: return "STATUS_IN_PAGE_ERROR";
                case 0xC0000008: return "STATUS_INVALID_HANDLE";
                case 0xC000000D: return "STATUS_INVALID_PARAMETER";
                case 0xC0000017: return "STATUS_NO_MEMORY";
                case 0xC000001D: return "STATUS_ILLEGAL_INSTRUCTION";
                case 0xC0000025: return "STATUS_NONCONTINUABLE_EXCEPTION";
                case 0xC0000026: return "STATUS_INVALID_DISPOSITION";
                case 0xC000008C: return "STATUS_ARRAY_BOUNDS_EXCEEDED";
                case 0xC000008D: return "STATUS_FLOAT_DENORMAL_OPERAND";
                case 0xC000008E: return "STATUS_FLOAT_DIVIDE_BY_ZERO";
                case 0xC000008F: return "STATUS_FLOAT_INEXACT_RESULT";
                case 0xC0000090: return "STATUS_FLOAT_INVALID_OPERATION";
                case 0xC0000091: return "STATUS_FLOAT_OVERFLOW";
                case 0xC0000092: return "STATUS_FLOAT_STACK_CHECK";
                case 0xC0000093: return "STATUS_FLOAT_UNDERFLOW";
                case 0xC0000094: return "STATUS_INTEGER_DIVIDE_BY_ZERO";
                case 0xC0000095: return "STATUS_INTEGER_OVERFLOW";
                case 0xC0000096: return "STATUS_PRIVILEGED_INSTRUCTION";
                case 0xC00000FD: return "STATUS_STACK_OVERFLOW";
                case 0xC0000135: return "STATUS_DLL_NOT_FOUND";
                case 0xC0000138: return "STATUS_ORDINAL_NOT_FOUND";
                case 0xC0000139: return "STATUS_ENTRYPOINT_NOT_FOUND";
                case 0xC000013A: return "STATUS_CONTROL_C_EXIT";
                case 0xC0000142: return "STATUS_DLL_INIT_FAILED";
                case 0xC00002B4: return "STATUS_FLOAT_MULTIPLE_FAULTS";
                case 0xC00002B5: return "STATUS_FLOAT_MULTIPLE_TRAPS";
                case 0xC00002C9: return "STATUS_REG_NAT_CONSUMPTION";
                case 0xC0000374: return "STATUS_HEAP_CORRUPTION";
                case 0xC0000409: return "STATUS_STACK_BUFFER_OVERRUN";
                case 0xC0000417: return "STATUS_INVALID_CRUNTIME_PARAMETER";
                case 0xC0000420: return "STATUS_ASSERTION_FAILURE";
                case 0xC00004A2: return "STATUS_ENCLAVE_VIOLATION";
            }

            return "UNKNOWN_EXCEPTION";
        }

        inline void translate_seh(unsigned int code, EXCEPTION_POINTERS* ep)
        {
            const char* code_name = translate_exception_code(code);

            char buffer[2048];

            std::snprintf(buffer, sizeof(buffer), // clang-format off
#if defined(MEM_ARCH_X86_64)
                "%s (0x%08X) at 0x%016llX\n"
                "RAX = 0x%016llX RBX = 0x%016llX RCX = 0x%016llX RDX = 0x%016llX\n"
                "RSP = 0x%016llX RBP = 0x%016llX RSI = 0x%016llX RDI = 0x%016llX\n"
                "R8  = 0x%016llX R9  = 0x%016llX R10 = 0x%016llX R11 = 0x%016llX\n"
                "R12 = 0x%016llX R13 = 0x%016llX R14 = 0x%016llX R15 = 0x%016llX\n",
                code_name, code,
                reinterpret_cast<DWORD64>(ep->ExceptionRecord->ExceptionAddress),
                ep->ContextRecord->Rax, ep->ContextRecord->Rbx, ep->ContextRecord->Rcx, ep->ContextRecord->Rdx,
                ep->ContextRecord->Rsp, ep->ContextRecord->Rbp, ep->ContextRecord->Rsi, ep->ContextRecord->Rdi,
                ep->ContextRecord->R8,  ep->ContextRecord->R9,  ep->ContextRecord->R10, ep->ContextRecord->R11,
                ep->ContextRecord->R12, ep->ContextRecord->R13, ep->ContextRecord->R14, ep->ContextRecord->R15
#else /*if defined(MEM_ARCH_X86)*/
                "%s (0x%08X) at 0x%08X\n"
                "EAX = 0x%08lX EBX = 0x%08lX ECX = 0x%08lX EDX = 0x%08lX\n"
                "ESP = 0x%08lX EBP = 0x%08lX ESI = 0x%08lX EDI = 0x%08lX\n",
                code_name, code,
                reinterpret_cast<DWORD>(ep->ExceptionRecord->ExceptionAddress),
                ep->ContextRecord->Eax, ep->ContextRecord->Ebx, ep->ContextRecord->Ecx, ep->ContextRecord->Edx,
                ep->ContextRecord->Esp, ep->ContextRecord->Ebp, ep->ContextRecord->Esi, ep->ContextRecord->Edi
#endif
            ); // clang-format on

            throw std::runtime_error(buffer);
        }
    } // namespace internal

#    if defined(_MSC_VER)
#        pragma warning(push)
#        pragma warning(disable : 4535) // warning C4535: calling _set_se_translator() requires /EHa
#    endif

    inline scoped_seh::scoped_seh()
        : old_handler_(_set_se_translator(&internal::translate_seh))
    {}

    inline scoped_seh::~scoped_seh()
    {
        _set_se_translator(old_handler_);
    }

#    if defined(_MSC_VER)
#        pragma warning(pop)
#    endif

#elif defined(__unix__)
    inline signal_handler::signal_handler()
        : sig_stack_(new char[MINSIGSTKSZ])
    {
        stack_t new_stack {};

        new_stack.ss_sp = sig_stack_.get();
        new_stack.ss_size = MINSIGSTKSZ;
        new_stack.ss_flags = 0;
        sigaltstack(&new_stack, &old_stack_);

        struct sigaction sa;

        sa.sa_sigaction = &sig_handler;
        sa.sa_flags = SA_ONSTACK | SA_SIGINFO;
        sigemptyset(&sa.sa_mask);

        sigaction(SIGSEGV, &sa, &old_actions_[0]);
        sigaction(SIGILL, &sa, &old_actions_[1]);
        sigaction(SIGFPE, &sa, &old_actions_[2]);
    }

    inline signal_handler::~signal_handler()
    {
        sigaction(SIGSEGV, &old_actions_[0], nullptr);
        sigaction(SIGILL, &old_actions_[1], nullptr);
        sigaction(SIGFPE, &old_actions_[2], nullptr);

        sigaltstack(&old_stack_, nullptr);
    }

    inline void signal_handler::sig_handler(int /*sig*/, siginfo_t* /*info*/, void* /*ucontext*/)
    {
        signal_handler* current = current_handler();

        if (!current)
        {
            std::abort();
        }

        siglongjmp(current->jmp_buffer_, 1);
    }

    inline signal_handler*& signal_handler::current_handler()
    {
        static thread_local signal_handler* current {nullptr};

        return current;
    }

    inline signal_handler::scoped_handler::scoped_handler(signal_handler* handler)
        : prev_(current_handler())
    {
        current_handler() = handler;
    }

    inline signal_handler::scoped_handler::~scoped_handler()
    {
        current_handler() = prev_;
    }
#endif
} // namespace mem

#endif // MEM_EXECUTION_HANDLER_BRICK_H
