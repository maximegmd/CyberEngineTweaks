#include <stdafx.h>

#include "Image.h"

void VirtualInputPatch(Image* apImage)
{
    const uint8_t payload[] = {
         0x8B, 0x44, 0x24, 0x54, 0x85, 0xC0, 0x75, 0x26
    };

    auto* pMemoryItor = apImage->pTextStart;
    auto* pEnd = apImage->pTextEnd;

    while (pMemoryItor + std::size(payload) < pEnd)
    {
        if (memcmp(pMemoryItor, payload, std::size(payload)) == 0)
        {
            DWORD oldProtect = 0;
            VirtualProtect(pMemoryItor, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
            pMemoryItor[0] = 0x36;
            pMemoryItor[1] = 0x8B;
            pMemoryItor[2] = 0x07;
            pMemoryItor[3] = 0x90;
            VirtualProtect(pMemoryItor, 8, oldProtect, nullptr);

            spdlog::info("Virtual Input Patch: success");

            return;
        }

        pMemoryItor++;
    }

    spdlog::warn("Virtual Input Patch: failed");

}
