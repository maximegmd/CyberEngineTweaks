#include "stdafx.h"

#include "GameDump.h"

namespace GameDump
{
bool DumpVTablesTask::Run()
{
    TiltedPhoques::Map<uintptr_t, std::string> vtableMap;

    auto ModuleBase = GetModuleHandle(nullptr);
    auto begin = reinterpret_cast<uintptr_t>(ModuleBase);
    const auto* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(ModuleBase);
    const auto* ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS*>(reinterpret_cast<const std::uint8_t*>(dosHeader) + dosHeader->e_lfanew);
    uintptr_t end = begin + ntHeader->OptionalHeader.SizeOfCode + ntHeader->OptionalHeader.SizeOfInitializedData;

    const auto* pRttiSystem = RED4ext::CRTTISystem::Get();

    auto dumpClass = [begin, end](auto& aVtableMap, RED4ext::CBaseRTTIType* apType)
    {
        uintptr_t vtable = *reinterpret_cast<uintptr_t*>(apType);
        const RED4ext::CName typeName = apType->GetName();
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
            const auto pMemory = std::make_unique<char[]>(size);

            memset(pMemory.get(), 0, size);

            apType->Construct(pMemory.get());

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

    pRttiSystem->types.for_each(
        [&dumpClass, &vtableMap](RED4ext::CName aName, RED4ext::CBaseRTTIType*& apType)
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
        Log::Info("{:016X},{}", key, value);
    }

    return true;
}
} // namespace GameDump
