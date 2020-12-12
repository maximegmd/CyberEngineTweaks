#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <cstdint>
#include <windows.h>
#include <DbgHelp.h>
#include <xutility>
#include <sstream>
#pragma comment( lib, "dbghelp.lib" )
#pragma comment(linker, "/DLL")

void Patch(HMODULE mod)
{
    IMAGE_NT_HEADERS* pHeader = ImageNtHeader(mod);
    auto* pSectionHeaders = (IMAGE_SECTION_HEADER*)(pHeader + 1);

    uint8_t* pImageBase = reinterpret_cast<uint8_t*>(mod);

    for (auto count = 0u; count < pHeader->FileHeader.NumberOfSections; ++count)
    {
        if (memcmp(pSectionHeaders->Name, ".text", 5) == 0)
        {
            auto* pMemoryItor = pImageBase + pSectionHeaders->VirtualAddress;
            auto* pEnd = pImageBase + pSectionHeaders->VirtualAddress + pSectionHeaders->Misc.VirtualSize;

            const uint8_t payload[] = {
                0x75, 0x30, 0x33, 0xC9, 0xB8, 0x01, 0x00, 0x00, 0x00, 0x0F, 0xA2, 0x8B, 0xC8, 0xC1, 0xF9, 0x08
            };

            while(pMemoryItor + std::size(payload) < pEnd)
            {
                if(memcmp(pMemoryItor, payload, std::size(payload)) == 0)
                {
                    DWORD oldProtect = 0;
                    VirtualProtect(pMemoryItor, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
                    *pMemoryItor = 0x74;
                    VirtualProtect(pMemoryItor, 8, oldProtect, nullptr);

                    return;
                }

                pMemoryItor++;
            }

            return;
        }

        ++pSectionHeaders;
    }
}


BOOL APIENTRY DllMain(HMODULE mod, DWORD ul_reason_for_call, LPVOID) {
    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        Patch(GetModuleHandleA(0));
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}
