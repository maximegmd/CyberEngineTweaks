#include "Image.h"
#include <windows.h>
#include <zlib.h>
#include <spdlog/spdlog.h>
#include <dbghelp.h>
#include <sstream>

Image::Image()
{
    auto* pImage = GetModuleHandleA(nullptr);

    IMAGE_NT_HEADERS* pHeader = ImageNtHeader(pImage);
    auto* pSectionHeaders = reinterpret_cast<IMAGE_SECTION_HEADER*>(pHeader + 1);

    base_address = reinterpret_cast<uintptr_t>(pImage);

    for (auto count = 0u; count < pHeader->FileHeader.NumberOfSections; ++count)
    {
        if (memcmp(pSectionHeaders->Name, ".text", 5) == 0)
        {
            pTextStart = reinterpret_cast<uint8_t*>(base_address + pSectionHeaders->VirtualAddress);
            pTextEnd = reinterpret_cast<uint8_t*>(base_address + pSectionHeaders->VirtualAddress + pSectionHeaders->Misc.VirtualSize);
        }

        ++pSectionHeaders;
    }

    uint32_t crc = crc32(0, pTextStart, pTextEnd - pTextStart);
    std::ostringstream oss;
    oss << crc;

    spdlog::info("Computed .text crc: {:X}", crc); 

    switch(crc)
    {
    case 3622375216:
        spdlog::info("\tResolved to version: 1.04");
        version = MakeVersion(1,4);
        break;
    case 0x20F87F01:
        spdlog::info("\tResolved to version: 1.05");
        version = MakeVersion(1, 5);
        break;
    default:
        spdlog::error("\tUnknown version, please update the mod");
        break;
    }
}
