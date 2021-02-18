#include <stdafx.h>

#include "CET.h"
#include "LuaVM.h"

#include <overlay/Overlay.h>

void LuaVM::HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    RED4ext::CString text("");
    apStack->unk30 = 0;
    apStack->unk38 = 0;
    const auto opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &text, nullptr);
    apStack->code++; // skip ParamEnd

    auto& console = CET::Get().GetOverlay().GetConsole();
    if (console.GameLogEnabled())
        spdlog::get("scripting")->info(text.c_str());
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
    return "";
}

void LuaVM::HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    uint64_t channel_hash = 0;
    apStack->unk30 = 0;
    apStack->unk38 = 0;
    uint8_t opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &channel_hash, nullptr);

    RED4ext::CString text("");
    apStack->unk30 = 0;
    apStack->unk38 = 0;
    opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &text, nullptr);

    apStack->code++; // skip ParamEnd

    auto& console = CET::Get().GetOverlay().GetConsole();
    if (console.GameLogEnabled())
    {
        auto consoleLogger = spdlog::get("scripting");

        std::string_view textSV = text.c_str();
        std::string_view channelSV = GetChannelStr(channel_hash);
        if (channelSV == "")
            consoleLogger->info("[?{0:x}] {}", channel_hash, textSV);
        else
            consoleLogger->info("[{}] {}", channelSV, textSV);
    }
    
    CET::Get().GetVM().m_logCount.fetch_add(1);
}

static std::string GetTDBDIDDebugString(TDBID tdbid)
{
    return (tdbid.unk5 == 0 && tdbid.unk7 == 0)
        ? fmt::format("<TDBID:{:08X}:{:02X}>",
            tdbid.name_hash, tdbid.name_length)
        : fmt::format("<TDBID:{:08X}:{:02X}:{:04X}:{:02X}>",
            tdbid.name_hash, tdbid.name_length, tdbid.unk5, tdbid.unk7);
}

void LuaVM::RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& aName)
{
    std::lock_guard<std::recursive_mutex> _{ m_tdbidLock };
    m_tdbidLookup[aValue] = { aBase, aName };
}

std::string LuaVM::GetTDBIDString(uint64_t aValue)
{
    std::lock_guard<std::recursive_mutex> _{ m_tdbidLock };
    auto it = m_tdbidLookup.find(aValue);
    auto end = m_tdbidLookup.end();
    if (it == end)
        return GetTDBDIDDebugString(TDBID{ aValue });
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

LuaVM::LuaVM(Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12, Options& aOptions)
    : m_scripting(aPaths, aBindings, aD3D12, aOptions)
    , m_d3d12(aD3D12)
{
    Hook(aOptions);

    m_connectUpdate = aD3D12.OnUpdate.Connect([this]() { Update(ImGui::GetIO().DeltaTime); });
}

LuaVM::~LuaVM()
{
    m_d3d12.OnUpdate.Disconnect(m_connectUpdate);
}

TDBID* LuaVM::HookTDBIDCtor(TDBID* apThis, const char* acpName)
{
    auto& luavm = CET::Get().GetVM();
    auto result = luavm.m_realTDBIDCtor(apThis, acpName);
    luavm.RegisterTDBIDString(apThis->value, 0, acpName);
    return result;
}

TDBID* LuaVM::HookTDBIDCtorCString(TDBID* apThis, const RED4ext::CString* acpName)
{
    auto& luavm = CET::Get().GetVM();
    auto result = luavm.m_realTDBIDCtorCString(apThis, acpName);
    luavm.RegisterTDBIDString(apThis->value, 0, acpName->c_str());
    return result;
}

TDBID* LuaVM::HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* acpName)
{
    auto& luavm = CET::Get().GetVM();
    auto result = luavm.m_realTDBIDCtorDerive(apBase, apThis, acpName);
    luavm.RegisterTDBIDString(apThis->value, apBase->value, std::string(acpName));
    return result;
}

struct UnknownString
{
    const char* string;
    uint32_t size;
};

TDBID* LuaVM::HookTDBIDCtorUnknown(TDBID* apThis, uint64_t aName)
{
    auto& luavm = CET::Get().GetVM();
    auto result = luavm.m_realTDBIDCtorUnknown(apThis, aName);
    UnknownString unknown;
    luavm.m_someStringLookup(&aName, &unknown);
    luavm.RegisterTDBIDString(apThis->value, 0, std::string(unknown.string, unknown.size));
    return result;
}

void LuaVM::HookTDBIDToStringDEBUG(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void* apResult, void*)
{
    TDBID tdbid;
    apStack->unk30 = 0;
    apStack->unk38 = 0;
    uint8_t opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &tdbid, nullptr);
    apStack->code++; // skip ParamEnd

    if (apResult)
    {
        std::string name = CET::Get().GetVM().GetTDBIDString(tdbid.value);
        RED4ext::CString s(name.c_str());
        *static_cast<RED4ext::CString*>(apResult) = s;
    }
}

void LuaVM::Hook(Options& aOptions)
{
    auto& gameImage = aOptions.GameImage;

    {
        const mem::pattern cPattern("40 53 48 83 EC 40 48 8B DA E8 ?? ?? ?? ?? 48 8B D0 48 8D 4C 24 20");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookLog, reinterpret_cast<void**>(&m_realLog)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook Log function!");
            else
                spdlog::info("Log function hook complete!");
        }
    }

    {
        const mem::pattern cPattern("48 89 5C 24 08 48 89 74 24 18 57 48 83 EC 40 48 8B 02 48 8D 3D ?? ?? ?? ?? 33 F6 4C 8D 44 24 58 48 89 74 24 58 45 33 C9 48 89 72 30 48 8B DA 48 89 72 38 0F B6 08 48 FF C0 48 89 02 8B C1 48 8B 4A 40 FF 14 C7 E8 ?? ?? ?? ?? 48 8B D0");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookLogChannel, reinterpret_cast<void**>(&m_realLogChannel)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook LogChannel function!");
            else
                spdlog::info("LogChannel function hook complete!");
        }
    }

    {
        const mem::pattern cPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 80 3A 00 48 8B FA");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTDBIDCtor, reinterpret_cast<void**>(&m_realTDBIDCtor)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook TDBID::ctor function!");
            else
                spdlog::info("TDBID::ctor function hook complete!");
        }
    }

    {
        const mem::pattern cPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 30 48 8B F1 48 8B DA 48 8B CA E8 ?? ?? ?? ?? 48 8B CB");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTDBIDCtorCString, reinterpret_cast<void**>(&m_realTDBIDCtorCString)) !=
                    MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook TDBID::ctor[CString] function!");
            else
                spdlog::info("TDBID::ctor[CString] function hook complete!");
        }
    }

    {
        const mem::pattern cPattern("48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 33 C0 4D 8B C8 48 8B F2 4D 85 C0 74 0F 41 38 00");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTDBIDCtorDerive, reinterpret_cast<void**>(&m_realTDBIDCtorDerive)) !=
                    MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook TDBID::ctor[Derive] function!");
            else
                spdlog::info("TDBID::ctor[Derive] function hook complete!");
        }
    }

    {
        const mem::pattern cPattern("48 89 5C 24 08 48 89 54 24 10 57 48 83 EC 50 48 8B F9 48 8D 54 24 20 48 8D 4C 24 68 E8");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTDBIDCtorUnknown, reinterpret_cast<void**>(&m_realTDBIDCtorUnknown)) !=
                    MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook TDBID::ctor[Unknown] function!");
            else
            {
                spdlog::info("TDBID::ctor[Unknown] function hook complete!");
                *reinterpret_cast<void**>(&m_someStringLookup) =
                    &pLocation[33] + *reinterpret_cast<int32_t*>(&pLocation[29]);
            }
        }
    }

    {
        const mem::pattern cPattern("48 BF 58 D1 78 A0 18 09 BA EC 75 16 48 8D 15 ?? ?? ?? ?? 48 8B CF E8 ?? ?? ?? ?? "
                                    "C6 05 ?? ?? ?? ?? 01 41 8B 06 39 05 ?? ?? ?? ?? 7F");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            pLocation = &pLocation[45] + static_cast<int8_t>(pLocation[44]);

            mem::region reg(pLocation, 45);
            const mem::pattern cSecondaryPattern(
                "48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 3D ?? ?? ?? ?? FF 75 ?? 48 8D 05");
            const mem::default_scanner cSecondaryScanner(cSecondaryPattern);

            pLocation = cSecondaryScanner(reg).as<uint8_t*>();

            if (pLocation)
            {
                pLocation = &pLocation[28] + *reinterpret_cast<int32_t*>(&pLocation[24]);
                if (MH_CreateHook(pLocation, &HookTDBIDToStringDEBUG,
                                  reinterpret_cast<void**>(&m_realTDBIDToStringDEBUG)) != MH_OK ||
                    MH_EnableHook(pLocation) != MH_OK)
                    spdlog::error("Could not hook TDBID::ToStringDEBUG function!");
                else
                    spdlog::info("TDBID::ToStringDEBUG function hook complete!");
            }
        }
    }
}
