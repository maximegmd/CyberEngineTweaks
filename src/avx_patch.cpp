#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <cstdint>
#include <windows.h>
#include <DbgHelp.h>
#include <spdlog/spdlog.h>

#include "Image.h"

void PatchAvx(Image* apImage)
{
    const uint8_t payload[] = {
         0x55, 0x48, 0x81 , 0xec , 0xa0 , 0x00 , 0x00 , 0x00 , 0x0f , 0x29 , 0x70 , 0xe8
    };

    auto* pMemoryItor = apImage->pTextStart;
    auto* pEnd = apImage->pTextEnd;

    while (pMemoryItor + std::size(payload) < pEnd)
    {
        if (memcmp(pMemoryItor, payload, std::size(payload)) == 0)
        {
            DWORD oldProtect = 0;
            VirtualProtect(pMemoryItor, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
            *pMemoryItor = 0xC3;
            VirtualProtect(pMemoryItor, 8, oldProtect, nullptr);

            spdlog::info("\tAVX Patch: success");

            return;
        }

        pMemoryItor++;
    }

    spdlog::warn("\tAVX Patch: failed");

}
