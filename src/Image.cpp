#include <stdafx.h>

#include "Image.h"

struct PdbInfo
{
    DWORD     Signature;
    BYTE      Guid[16];
    DWORD     Age;
    char      PdbFileName[1];
};

void Image::Initialize()
{
    static uint8_t s_Guid104[] = { 0x2B, 0x4E, 0x65, 0x3D, 0xD4, 0x68, 0xC7, 0x42, 0xBF, 0xC9, 0x58, 0xDC, 0x38, 0xD4, 0x2A, 0x36 };
    static uint8_t s_Guid105[] = { 0x93, 0x5B, 0x36, 0x35, 0xDF, 0xA8, 0xE7, 0x41, 0x91, 0x8A, 0x64, 0x64, 0xF7, 0xA4, 0xF0, 0x8E };
    static uint8_t s_Guid106[] = { 0x67, 0xFB, 0x96, 0x6B, 0xAA, 0x3D, 0x57, 0x4E, 0x93, 0x8F, 0x1C, 0xC5, 0x85, 0xc6, 0xF5, 0x29 };

    auto* pImage = GetModuleHandle(nullptr);
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

    auto* pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(base_address);
    auto* pFileHeader = reinterpret_cast<IMAGE_FILE_HEADER*>(base_address + pDosHeader->e_lfanew + 4);
    auto* pOptionalHeader = reinterpret_cast<IMAGE_OPTIONAL_HEADER*>(((char*)pFileHeader) + sizeof(IMAGE_FILE_HEADER));
    auto* pDataDirectory = &pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    auto* pDebugDirectory = reinterpret_cast<IMAGE_DEBUG_DIRECTORY*>(base_address + pDataDirectory->VirtualAddress);

    // Check to see that the data has the right type
    if (IMAGE_DEBUG_TYPE_CODEVIEW == pDebugDirectory->Type)
    {
        PdbInfo* pdb_info = (PdbInfo*)(base_address + pDebugDirectory->AddressOfRawData);
        if (0 == memcmp(&pdb_info->Signature, "RSDS", 4))
        {
            if (memcmp(&pdb_info->Guid, s_Guid104, 16) == 0)
                version = MakeVersion(1, 4);
            else if (memcmp(&pdb_info->Guid, s_Guid105, 16) == 0)
                version = MakeVersion(1, 5);
            else if (memcmp(&pdb_info->Guid, s_Guid106, 16) == 0)
                version = MakeVersion(1, 6);
            else
            {
                for (auto c : pdb_info->Guid)
                    spdlog::info("{:X}", (uint32_t)c);
            }
        }
    }
}
