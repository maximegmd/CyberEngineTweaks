#include <stdafx.h>

#include "GameDump.h"

#include <stack>
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

    auto rttiSystem = RED4ext::CRTTISystem::Get();
    auto* scriptable = rttiSystem->GetClass("IScriptable");

    auto dumpClass = [scriptable, begin, end](auto& vtableMap, RED4ext::IRTTIType* type) {
        uintptr_t vtable = *(uintptr_t*)type;
        RED4ext::CName typeName;
        type->GetName(typeName);
        std::string name = typeName.ToString();
        if (vtable >= begin && vtable <= end)
        {
            vtableMap.emplace(vtable, "VT_RTTI_" + name);
        }

        // Construct an empty instance of this class and dump that
        if (type->GetType() == RED4ext::ERTTIType::Class)
        {
            auto classType = static_cast<RED4ext::CClass*>(type);
            uint32_t size = type->GetSize();

            // We aren't borrowing the game's allocator on purpose because some classes have Abstract
            // allocators and they assert
            std::unique_ptr<char[]> mem = std::make_unique<char[]>(size);

            memset(mem.get(), 0, size);

            type->Init(mem.get());

            if (size >= sizeof(uintptr_t))
            {
                vtable = *(uintptr_t*)mem.get();

                if (vtable >= begin && vtable <= end)
                {
                    vtableMap.emplace(vtable, "VT_" + name);
                }
            }

            // Lets just leak memory from nested objects for now, this is broken on certain classes,
            // havent determined why
            // type->Destroy(buffer);
        }
    };

    rttiSystem->types.for_each([&dumpClass, &vtableMap, begin, end](RED4ext::CName n, RED4ext::IRTTIType*& type) {
        uintptr_t vtable = *(uintptr_t*)type;

        std::string name = n.ToString();

        dumpClass(vtableMap, type);

        if (type->GetType() == RED4ext::ERTTIType::Class)
        {
            auto parent = static_cast<RED4ext::CClass*>(type)->parent;
            while (parent)
            {
                dumpClass(vtableMap, parent);

                parent = parent->parent;
                if (!parent || parent->GetType() != RED4ext::ERTTIType::Class)
                {
                    break;
                }
            }
        }
    });

    for (auto p : vtableMap)
    {
        spdlog::info("{:016X},{}", p.first, p.second);
    }
}

void DumpVTablesTask::Dispose()
{
    delete this;
}

} // namespace GameDump
