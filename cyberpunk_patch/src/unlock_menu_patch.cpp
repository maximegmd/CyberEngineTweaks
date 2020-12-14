#include "Image.h"
#include <spdlog/spdlog.h>


void UnlockMenuPatch(Image* apImage)
{
    uint8_t* pAddress = nullptr;

    if (apImage->version == Image::MakeVersion(1, 4))
    {
        pAddress = reinterpret_cast<uint8_t*>(apImage->base_address + 0x207c4b);
    }
    else
    {
        spdlog::warn("\tUnlock menu patch: failed, unknown version");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pAddress, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    *pAddress = 0;
    VirtualProtect(pAddress, 8, oldProtect, nullptr);

    spdlog::info("\tUnlock menu patch: success");
}
