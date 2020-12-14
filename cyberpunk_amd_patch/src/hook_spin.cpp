#include <windows.h>

#include "Image.h"
#include <cstring>
#include <spdlog/spdlog.h>

struct Mutex
{
    uint32_t unk0;
    HANDLE handle;
    uint8_t pad4[0x40 - 0x10];
    int32_t unk40;
};

static_assert(offsetof(Mutex, handle) == 8);
static_assert(offsetof(Mutex, unk40) == 0x40);

void HookSpin(Mutex* apMutex)
{
    auto spinCount = apMutex->unk40;
    while (true)
    {
        if (apMutex->unk0 > 0)
        {
            const auto oldValue = apMutex->unk0;

            if (oldValue == _InterlockedCompareExchange(&apMutex->unk0, apMutex->unk0 - 1, apMutex->unk0))
                return;
        }

        const auto oldSpin = spinCount;
        spinCount--;

        if (oldSpin <= 0)
        {
            const auto result = _InterlockedExchangeAdd(&apMutex->unk0, 0xFFFFFFFF);
            if((int(result) - 1) < 0)
                WaitForSingleObject(apMutex->handle, 0xFFFFFFFF);
            return;
        }
    }
}

void PatchSpin(Image* apImage)
{
    if (apImage->version != Image::MakeVersion(1, 4))
    {
        spdlog::info("\tSpin Patch: failed");
        return;
    }

    auto* pFuncPtr = &HookSpin;

    auto addr = (uint8_t*)(0x2AEEC70 + apImage->base_address);
    DWORD oldProtect = 0;
    VirtualProtect(addr, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);

    // mov rax, HookSpin
    addr[0] = 0x48;
    addr[1] = 0xB8;
    std::memcpy(addr + 2, &pFuncPtr, 8);

    // jmp rax
    addr[10] = 0xFF;
    addr[11] = 0xE0;
    VirtualProtect(addr, 32, oldProtect, nullptr);

    spdlog::info("\tSpin Patch: success");
}
