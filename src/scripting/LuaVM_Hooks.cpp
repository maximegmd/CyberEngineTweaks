#include <stdafx.h>

#include "LuaVM.h"
#include "Scripting.h"

#include <Image.h>
#include <Pattern.h>

#include <console/Console.h>

struct REDScriptContext
{
};

struct ScriptStack
{
    uint8_t* m_code;
    uint8_t pad[0x28];
    void* unk30;
    void* unk38;
    REDScriptContext* m_context;
};
static_assert(offsetof(ScriptStack, m_context) == 0x40);

static TScriptCall** GetScriptCallArray()
{
    static uint8_t* pLocation = FindSignature({ 0x4C, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x89, 0x42, 0x38, 0x49, 0x8B, 0xF8, 0x48, 0x8B, 0x02, 0x4C, 0x8D, 0x44, 0x24, 0x20, 0xC7 }) + 3;
    static uintptr_t finalLocation = (uintptr_t)pLocation + 4 + *reinterpret_cast<uint32_t*>(pLocation);

    return reinterpret_cast<TScriptCall**>(finalLocation);
}

void LuaVM::HookLog(REDScriptContext*, ScriptStack* pStack, void*, void*)
{
    RED4ext::CString text("");
    pStack->unk30 = nullptr;
    pStack->unk38 = nullptr;
    auto opcode = *(pStack->m_code++);
    GetScriptCallArray()[opcode](pStack->m_context, pStack, &text, nullptr);
    pStack->m_code++; // skip ParamEnd
    
    if (Options::Get().Console)
        Console::Get().GameLog(text.c_str());

    Get().PostInitialize();
}

static const char* GetChannelStr(uint64_t hash)
{
    switch (hash)
    {
#define HASH_CASE(x) case RED4ext::FNV1a(x): return x
        HASH_CASE("AI");
        HASH_CASE("AICover");
        HASH_CASE("ASSERT");
        HASH_CASE("Damage");
        HASH_CASE("DevelopmentManager");
        HASH_CASE("Device");
        HASH_CASE("Items");
        HASH_CASE("ItemManager");
        HASH_CASE("Puppet");
        HASH_CASE("Scanner");
        HASH_CASE("Stats");
        HASH_CASE("StatPools");
        HASH_CASE("Strike");
        HASH_CASE("TargetManager");
        HASH_CASE("Test");
        HASH_CASE("UI");
        HASH_CASE("Vehicles");
#undef HASH_CASE
    }
    return nullptr;
}

void LuaVM::HookLogChannel(REDScriptContext*, ScriptStack* pStack, void*, void*)
{
    uint8_t opcode;

    uint64_t channel_hash = 0;
    pStack->unk30 = nullptr;
    pStack->unk38 = nullptr;
    opcode = *(pStack->m_code++);
    GetScriptCallArray()[opcode](pStack->m_context, pStack, &channel_hash, nullptr);

    RED4ext::CString text("");
    pStack->unk30 = nullptr;
    pStack->unk38 = nullptr;
    opcode = *(pStack->m_code++);
    GetScriptCallArray()[opcode](pStack->m_context, pStack, &text, nullptr);

    pStack->m_code++; // skip ParamEnd
    
    auto channel_str = GetChannelStr(channel_hash);
    std::string channel = channel_str == nullptr
        ? "?" + std::to_string(channel_hash)
        : std::string(channel_str);
    if (Options::Get().Console)
        Console::Get().GameLog("[" + channel + "] " +text.c_str());
    

    Get().PostInitialize();
}

static std::string GetTDBDIDDebugString(TDBID tdbid)
{
    return (tdbid.unk5 == 0 && tdbid.unk7 == 0)
        ? fmt::format("<TDBID:{:08X}:{:02X}>",
            tdbid.name_hash, tdbid.name_length)
        : fmt::format("<TDBID:{:08X}:{:02X}:{:04X}:{:02X}>",
            tdbid.name_hash, tdbid.name_length, tdbid.unk5, tdbid.unk7);
}

void LuaVM::RegisterTDBIDString(uint64_t value, uint64_t base, const std::string& name)
{
    std::lock_guard<std::recursive_mutex> _{ m_tdbidLock };
    m_tdbidLookup[value] = { base, name };
}

std::string LuaVM::GetTDBIDString(uint64_t value)
{
    std::lock_guard<std::recursive_mutex> _{ m_tdbidLock };
    auto it = m_tdbidLookup.find(value);
    auto end = m_tdbidLookup.end();
    if (it == end)
        return GetTDBDIDDebugString(TDBID{ value });
    std::string string = (*it).second.name;
    uint64_t base = (*it).second.base;
    while (base)
    {
        it = m_tdbidLookup.find(base);
        if (it == end)
        {
            string.insert(0, GetTDBDIDDebugString(TDBID{ base }));
            break;
        }
        string.insert(0, (*it).second.name);
        base = (*it).second.base;
    }
    return string;
}

TDBID* LuaVM::HookTDBIDCtor(TDBID* pThis, const char* name)
{
    auto& luavm = Get();
    auto result = luavm.m_realTDBIDCtor(pThis, name);
    luavm.RegisterTDBIDString(pThis->value, 0, name);
    return result;
}

TDBID* LuaVM::HookTDBIDCtorCString(TDBID* pThis, const RED4ext::CString* name)
{
    auto& luavm = Get();
    auto result = luavm.m_realTDBIDCtorCString(pThis, name);
    luavm.RegisterTDBIDString(pThis->value, 0, name->c_str());
    return result;
}

TDBID* LuaVM::HookTDBIDCtorDerive(TDBID* pBase, TDBID* pThis, const char* name)
{
    auto& luavm = Get();
    auto result = luavm.m_realTDBIDCtorDerive(pBase, pThis, name);
    luavm.RegisterTDBIDString(pThis->value, pBase->value, std::string(name));
    return result;
}

struct UnknownString
{
    const char* string;
    uint32_t size;
};

TDBID* LuaVM::HookTDBIDCtorUnknown(TDBID* pThis, uint64_t name)
{
    auto& luavm = Get();
    auto result = luavm.m_realTDBIDCtorUnknown(pThis, name);
    UnknownString unknown;
    luavm.m_someStringLookup(&name, &unknown);
    luavm.RegisterTDBIDString(pThis->value, 0, std::string(unknown.string, unknown.size));
    return result;
}

void LuaVM::HookTDBIDToStringDEBUG(REDScriptContext*, ScriptStack* pStack, void* result, void*)
{
    uint8_t opcode;

    TDBID tdbid;
    pStack->unk30 = nullptr;
    pStack->unk38 = nullptr;
    opcode = *(pStack->m_code++);
    GetScriptCallArray()[opcode](pStack->m_context, pStack, &tdbid, nullptr);
    pStack->m_code++; // skip ParamEnd

    if (result)
    {
        std::string name = Get().GetTDBIDString(tdbid.value);
        RED4ext::CString s(name.c_str());
        *static_cast<RED4ext::CString*>(result) = s;
    }
}

void LuaVM::Hook()
{
    uint8_t* pLocation = FindSignature({
        0x40, 0x53, 0x48, 0x83, 0xEC, 0x40, 0x48, 0x8B,
        0xDA, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B,
        0xD0, 0x48, 0x8D, 0x4C, 0x24, 0x20
    });

    if(pLocation)
    {
        if (MH_CreateHook(pLocation, &HookLog, reinterpret_cast<void**>(&m_realLog)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook Log function!");
        else
            spdlog::info("Log function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x40, 0x48,
        0x8B, 0x02, 0x48, 0x8D, 0x3D, 0xCC, 0xCC, 0xCC,
        0xCC, 0x33, 0xF6, 0x4C, 0x8D, 0x44, 0x24, 0x58,
        0x48, 0x89, 0x74, 0x24, 0x58, 0x45, 0x33, 0xC9,
        0x48, 0x89, 0x72, 0x30, 0x48, 0x8B, 0xDA, 0x48,
        0x89, 0x72, 0x38, 0x0F, 0xB6, 0x08, 0x48, 0xFF,
        0xC0, 0x48, 0x89, 0x02, 0x8B, 0xC1, 0x48, 0x8B,
        0x4A, 0x40, 0xFF, 0x14, 0xC7, 0xE8, 0xCC, 0xCC,
        0xCC, 0xCC, 0x48, 0x8B, 0xD0
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookLogChannel, reinterpret_cast<void**>(&m_realLogChannel)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook LogChannel function!");
        else
            spdlog::info("LogChannel function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x40, 0x80,
        0x3A, 0x00, 0x48, 0x8B, 0xFA
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtor, reinterpret_cast<void**>(&m_realTDBIDCtor)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook TDBID::ctor function!");
        else
            spdlog::info("TDBID::ctor function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48,
        0x8B, 0xF1, 0x48, 0x8B, 0xDA, 0x48, 0x8B, 0xCA,
        0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCB
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtorCString, reinterpret_cast<void**>(&m_realTDBIDCtorCString)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook TDBID::ctor[CString] function!");
        else
            spdlog::info("TDBID::ctor[CString] function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74,
        0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x33,
        0xC0, 0x4D, 0x8B, 0xC8, 0x48, 0x8B, 0xF2, 0x4D,
        0x85, 0xC0, 0x74, 0x0F, 0x41, 0x38, 0x00,
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtorDerive, reinterpret_cast<void**>(&m_realTDBIDCtorDerive)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook TDBID::ctor[Derive] function!");
        else
            spdlog::info("TDBID::ctor[Derive] function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x54,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x50, 0x48,
        0x8B, 0xF9, 0x48, 0x8D, 0x54, 0x24, 0x20, 0x48,
        0x8D, 0x4C, 0x24, 0x68, 0xE8
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtorUnknown, reinterpret_cast<void**>(&m_realTDBIDCtorUnknown)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook TDBID::ctor[Unknown] function!");
        else
        {
            spdlog::info("TDBID::ctor[Unknown] function hook complete!");
            *reinterpret_cast<void**>(&m_someStringLookup) = &pLocation[33] + *reinterpret_cast<int32_t*>(&pLocation[29]);
        }
    }

    pLocation = FindSignature({
        0x48, 0xBF, 0x58, 0xD1, 0x78, 0xA0, 0x18, 0x09,
        0xBA, 0xEC, 0x75, 0x16, 0x48, 0x8D, 0x15, 0xCC,
        0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCF, 0xE8, 0xCC,
        0xCC, 0xCC, 0xCC, 0xC6, 0x05, 0xCC, 0xCC, 0xCC,
        0xCC, 0x01, 0x41, 0x8B, 0x06, 0x39, 0x05, 0xCC,
        0xCC, 0xCC, 0xCC, 0x7F
        });

    if (pLocation)
    {
        pLocation = &pLocation[45] + static_cast<int8_t>(pLocation[44]);
        pLocation = FindSignature(pLocation, pLocation + 45, {
            0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8,
            0xCC, 0xCC, 0xCC, 0xCC, 0x83, 0x3D, 0xCC, 0xCC,
            0xCC, 0xCC, 0xFF, 0x75, 0xCC, 0x48, 0x8D, 0x05,
            });
        if (pLocation)
        {
            pLocation = &pLocation[28] + *reinterpret_cast<int32_t*>(&pLocation[24]);
            if (MH_CreateHook(pLocation, &HookTDBIDToStringDEBUG, reinterpret_cast<void**>(&m_realTDBIDToStringDEBUG)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook TDBID::ToStringDEBUG function!");
            else
                spdlog::info("TDBID::ToStringDEBUG function hook complete!");
        }
    }
}
