#include <stdafx.h>

#include "CET.h"
#include "LuaVM.h"

#include <overlay/Overlay.h>
#include <reverse/RTTILocator.h>

void LuaVM::HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    static RTTILocator s_stringLocator("String");

    RED4ext::CString text{};

    apStack->unk30 = 0;
    apStack->unk38 = 0;

    RED4ext::ScriptRef<RED4ext::CString> ref;
    ref.innerType = s_stringLocator;
    ref.ref = &text;
    ref.innerType->GetName(ref.hash);

    apStack->currentParam++;
    const auto opcode = *(apStack->code++);

    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &ref, &ref);
    apStack->code++; // skip ParamEnd

    auto& console = CET::Get().GetOverlay().GetConsole();
    if (console.GameLogEnabled())
        spdlog::get("scripting")->info(ref.ref->c_str());
}

void LuaVM::HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    static RTTILocator s_stringLocator("String");
    static RED4ext::CName s_debugChannel("DEBUG");

    RED4ext::CName channel;
    apStack->unk30 = 0;
    apStack->unk38 = 0;
    uint8_t opcode = *(apStack->code++);
    apStack->currentParam++;

    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &channel, nullptr);

    RED4ext::CString text{};
    RED4ext::ScriptRef<RED4ext::CString> ref;
    ref.innerType = s_stringLocator;
    ref.ref = &text;
    ref.innerType->GetName(ref.hash);

    apStack->currentParam++;

    apStack->unk30 = 0;
    apStack->unk38 = 0;
    opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &ref, &ref);

    apStack->code++; // skip ParamEnd

    auto& console = CET::Get().GetOverlay().GetConsole();
    if (console.GameLogEnabled() || channel == s_debugChannel)
    {
        auto consoleLogger = spdlog::get("scripting");

        std::string_view textSV = ref.ref->c_str();
        std::string_view channelSV = channel.ToString();
        if (channelSV == "")
            consoleLogger->info("[?{0:x}] {}", channel.hash, textSV);
        else
            consoleLogger->info("[{}] {}", channelSV, textSV);
    }
    
    CET::Get().GetVM().m_logCount.fetch_add(1);
}

LuaVM::LuaVM(Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12, Options& aOptions)
    : m_scripting(aPaths, aBindings, aD3D12, aOptions)
    , m_d3d12(aD3D12)
    , m_lastframe(std::chrono::high_resolution_clock::now())
{
    Hook(aOptions);

    m_connectUpdate = aD3D12.OnUpdate.Connect([this]() { Draw(); });
}

LuaVM::~LuaVM()
{
    m_d3d12.OnUpdate.Disconnect(m_connectUpdate);
}

TDBID* LuaVM::HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* acpName)
{
    auto& luavm = CET::Get().GetVM();
    auto result = luavm.m_realTDBIDCtorDerive(apBase, apThis, acpName);
    luavm.RegisterTDBIDString(apThis->value, apBase->value, std::string(acpName));
    return result;
}

void LuaVM::HookTDBIDToStringDEBUG(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void* apResult, void*)
{
    TDBID tdbid;
    apStack->unk30 = 0;
    apStack->unk38 = 0;
    apStack->currentParam++;
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

bool LuaVM::HookRunningStateRun(uintptr_t aThis, uintptr_t aApp)
{
    auto& luavm = CET::Get().GetVM();

    const auto cNow = std::chrono::high_resolution_clock::now();
    const auto cDelta = cNow - luavm.m_lastframe;
    auto cSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(cDelta);

    luavm.Update(cSeconds.count());

    luavm.m_lastframe = cNow;

    return luavm.m_realRunningStateRun(aThis, aApp);
}

uintptr_t LuaVM::HookSetLoadingState(uintptr_t aThis, int aState)
{
    auto& luavm = CET::Get().GetVM();

    static std::once_flag s_initBarrier;

    if (aState == 2)
    {
        std::call_once(s_initBarrier, [&luavm]()
        {
            luavm.PostInitializeStage2();
        });
    }

    return luavm.m_realSetLoadingState(aThis, aState);
}

uint64_t LuaVM::HookTweakDBLoad(uintptr_t aThis, uintptr_t aParam)
{
    auto& luavm = CET::Get().GetVM();

    const auto ret = luavm.m_realTweakDBLoad(aThis, aParam);

    luavm.PostInitializeStage1();

    return ret;
}

void LuaVM::Hook(Options& aOptions)
{
    auto& gameImage = aOptions.GameImage;

    {
        const mem::pattern cPattern("40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C  24 40 E8");
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
        const mem::pattern cPattern("4C 8B DC 49 89 5B 08 49  89 73 18 57 48 83 EC 70 48 8B 02 ? ? ? ? ? ? ? FE 42 62 4D 8D 43 10 33 FF 45 33 C9 49 89  7B 10 48 8B DA 48 89 7A");
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
        const mem::pattern cPattern("40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA");
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
        const mem::pattern cPattern("40 53 48 83 EC 20 48 8B 0D ? ? ? ? 48 8B DA E8 ? ? ? ? 84 C0");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookRunningStateRun, reinterpret_cast<void**>(&m_realRunningStateRun)) !=
                    MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook RunningState::Run function!");
            else
            {
                spdlog::info("RunningState::Run function hook complete!");
            }
        }
    }

    {
        const mem::pattern cPattern("48 BF 58 D1 78 A0 18 09 BA EC 75 16 48 8D 15 ? ? ? ? 48 8B CF E8 ? ? ? ? C6 05 ?? "
                                    "?? ?? ?? 01 41 8B 06 39 05 ? ? ? ? 7F");
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
    
    {
        const mem::pattern cPattern("48 89 5C 24 10 48 89 7C 24 18 4C 89 74 24 20 55 48 8B EC 48 83 EC 70 48");
        const mem::default_scanner cScanner(cPattern);
        uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTweakDBLoad, reinterpret_cast<void**>(&m_realTweakDBLoad)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("Could not hook TweakDB::Load function!");
            else
            {
                spdlog::info("TweakDB::Load function hook complete!");
            }
        }
    }

    // Disable SetLoadingState hook temporarily and get back to log count workaround
    // as it introduces major breaking change for onInit handler.
    //{
    //    const mem::pattern cPattern("48 89 5C 24 18 89 54 24 10 57 48 83 EC 20 48 8B D9 C7");
    //    const mem::default_scanner cScanner(cPattern);
    //    uint8_t* pLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();
    //
    //    if (pLocation)
    //    {
    //        if (MH_CreateHook(pLocation, &HookSetLoadingState, reinterpret_cast<void**>(&m_realSetLoadingState)) != MH_OK
    //         || MH_EnableHook(pLocation) != MH_OK)
    //            spdlog::error("Could not hook SetLoadingState function!");
    //        else
    //        {
    //            spdlog::info("SetLoadingState function hook complete!");
    //        }
    //    }
    //}

    
}
