#include <stdafx.h>

#include "GameDump.h"

#include <unordered_map>

namespace GameDump
{
void DumpVTablesTask::Run()
{
    std::unordered_map<uintptr_t, std::string> vtableMap;

    HMODULE ModuleBase = GetModuleHandle(nullptr);
    uintptr_t begin = reinterpret_cast<uintptr_t>(ModuleBase);
    const IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(ModuleBase);
    const IMAGE_NT_HEADERS* ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>(
        reinterpret_cast<const std::uint8_t*>(dosHeader) + dosHeader->e_lfanew);
    uintptr_t end = begin + ntHeader->OptionalHeader.SizeOfCode + ntHeader->OptionalHeader.SizeOfInitializedData;

    auto* pRttiSystem = RED4ext::CRTTISystem::Get();

    auto dumpClass = [begin, end](auto& aVtableMap, RED4ext::IRTTIType* apType)
    {
        uintptr_t vtable = *reinterpret_cast<uintptr_t*>(apType);
        RED4ext::CName typeName;
        apType->GetName(typeName);
        const std::string name = typeName.ToString();
        if (vtable >= begin && vtable <= end)
        {
            aVtableMap.emplace(vtable, "VT_RTTI_" + name);
        }

        // Construct an empty instance of this class and dump that
        if (apType->GetType() == RED4ext::ERTTIType::Class)
        {
            const uint32_t size = apType->GetSize();

            // We aren't borrowing the game's allocator on purpose because some classes have Abstract
            // allocators and they assert
            const std::unique_ptr<char[]> pMemory = std::make_unique<char[]>(size);

            memset(pMemory.get(), 0, size);

            apType->Init(pMemory.get());

            if (size >= sizeof(uintptr_t))
            {
                vtable = *reinterpret_cast<uintptr_t*>(pMemory.get());

                if (vtable >= begin && vtable <= end)
                {
                    aVtableMap.emplace(vtable, "VT_" + name);
                }
            }

            // Lets just leak memory from nested objects for now, this is broken on certain classes,
            // havent determined why
            // type->Destroy(buffer);
        }
    };

    pRttiSystem->types.for_each([&dumpClass, &vtableMap](RED4ext::CName aName, RED4ext::IRTTIType*& apType)
    {
        TP_UNUSED(aName);

        dumpClass(vtableMap, apType);

        if (apType->GetType() == RED4ext::ERTTIType::Class)
        {
            auto* pParent = static_cast<RED4ext::CClass*>(apType)->parent;
            while (pParent)
            {
                dumpClass(vtableMap, pParent);

                pParent = pParent->parent;
                if (!pParent || pParent->GetType() != RED4ext::ERTTIType::Class)
                {
                    break;
                }
            }
        }
    });

    for (auto& [key, value] : vtableMap)
    {
        spdlog::info("{:016X},{}", key, value);
    }
}
} // namespace GameDump
