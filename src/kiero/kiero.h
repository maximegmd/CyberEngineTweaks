#pragma once

#include <cstdint>

#define KIERO_VERSION "1.2.10-cet_1.0"

#if defined(_M_X64)
    #define KIERO_ARCH_X86 0
    #define KIERO_ARCH_X64 1
#else
    #define KIERO_ARCH_X86 1
    #define KIERO_ARCH_X64 0
#endif

#if KIERO_ARCH_X64
typedef uint64_t uint150_t;
#else
typedef uint32_t uint150_t;
#endif

namespace kiero
{
    struct Status
    {
        enum Enum
        {
            UnknownError = -1,
            NotSupportedError = -2,
            ModuleNotFoundError = -3,

            AlreadyInitializedError = -4,
            NotInitializedError = -5,

            Success = 0,
        };
    };

    Status::Enum init();
    void shutdown();

    Status::Enum bind(uint16_t index, void** original, void* function);
    void unbind(uint16_t index);

    void** getSwapChainVtable();
    void** getCommandListVtable();
    void** getCommandQueueVtable();

    uint150_t* getMethodsTable();
    uintptr_t getCommandQueueOffset();
    bool isDownLevelDevice();
}