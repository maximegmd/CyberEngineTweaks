#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <cstdint>
#include <windows.h>
#include <DbgHelp.h>
#include <spdlog/spdlog.h>
#include <filesystem>

#include "Image.h"

void SmtAmdPatch(Image* apImage)
{
    const uint8_t payload[] = {
        0x75, 0xCC, 0x33, 0xC9, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0xA2, 0x8B, 0xC8, 0xC1, 0xF9, 0x08
    };

    auto* pMemoryItor = apImage->pTextStart;
    auto* pEnd = apImage->pTextEnd;

    while(pMemoryItor + std::size(payload) < pEnd)
    {
        if(memcmp(pMemoryItor, payload, std::size(payload)) == 0)
        {
            DWORD oldProtect = 0;
            VirtualProtect(pMemoryItor, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
            *pMemoryItor = 0xEB;
            VirtualProtect(pMemoryItor, 8, oldProtect, nullptr);

            spdlog::info("\tAMD SMT Patch: success");

            return;
        }

        pMemoryItor++;
    }

    spdlog::warn("\tAMD SMT Patch: failed");
}
