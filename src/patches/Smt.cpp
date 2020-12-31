#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

void SmtAmdPatch(Image* apImage)
{
    auto* pMemoryItor = FindSignature({
        0x75, 0xCC, 0x33, 0xC9, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0xA2, 0x8B, 0xC8, 0xC1, 0xF9, 0x08
        });

    if(pMemoryItor)
    {
        DWORD oldProtect = 0;
        VirtualProtect(pMemoryItor, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
        *pMemoryItor = 0xEB;
        VirtualProtect(pMemoryItor, 8, oldProtect, nullptr);

        spdlog::info("AMD SMT Patch: success");

        return;
    }

    spdlog::warn("AMD SMT Patch: failed");
}
