#include <stdafx.h>

#include "CET.h"
#include "LuaVM.h"

#include <overlay/Overlay.h>
#include <reverse/RTTILocator.h>

static LuaVM* s_vm{ nullptr };

void LuaVM::HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    static RTTILocator s_stringLocator("String");

    RED4ext::CString text{};

    apStack->data = 0;
    apStack->dataType = 0;

    RED4ext::ScriptRef<RED4ext::CString> ref;
    ref.innerType = s_stringLocator;
    ref.ref = &text;
    ref.innerType->GetName(ref.hash);

    apStack->currentParam++;
    const auto opcode = *(apStack->code++);

    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &ref, &ref);
    apStack->code++; // skip ParamEnd

    spdlog::get("gamelog")->info("[GENERAL] {}", ref.ref->c_str());
}

void LuaVM::HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    static RTTILocator s_stringLocator("String");
    static RED4ext::CName s_debugChannel("DEBUG");
    static RED4ext::CName s_assertionChannel("ASSERT");

    RED4ext::CName channel;
    apStack->data = 0;
    apStack->dataType = 0;
    uint8_t opcode = *(apStack->code++);
    apStack->currentParam++;

    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &channel, nullptr);

    RED4ext::CString text{};
    RED4ext::ScriptRef<RED4ext::CString> ref;
    ref.innerType = s_stringLocator;
    ref.ref = &text;
    ref.hash = ref.innerType->GetName();

    apStack->currentParam++;

    apStack->data = nullptr;
    apStack->dataType = nullptr;
    opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &ref, &ref);

    apStack->code++; // skip ParamEnd

    if (channel == s_debugChannel)
        spdlog::get("scripting")->debug("{}", ref.ref->c_str());

    std::string_view channelSV = channel.ToString();
    if (channelSV.empty())
        spdlog::get("gamelog")->info("[?{:X}] {}", channel.hash, ref.ref->c_str());
    else
    {
        if (channel == s_debugChannel)
            spdlog::get("gamelog")->debug("[{}] {}", channelSV, ref.ref->c_str());
        else if (channel == s_assertionChannel)
            spdlog::get("gamelog")->warn("[{}] {}", channelSV, ref.ref->c_str());
        else
            spdlog::get("gamelog")->info("[{}] {}", channelSV, ref.ref->c_str());
    }

    s_vm->m_logCount.fetch_add(1);
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
    const auto result = s_vm->m_realTDBIDCtorDerive(apBase, apThis, acpName);
    s_vm->RegisterTDBIDString(apThis->value, apBase->value, std::string(acpName));
    return result;
}

void LuaVM::HookTDBIDToStringDEBUG(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void* apResult, void*)
{
    TDBID tdbid;
    apStack->data = 0;
    apStack->dataType = 0;
    apStack->currentParam++;
    uint8_t opcode = *(apStack->code++);
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &tdbid, nullptr);
    apStack->code++; // skip ParamEnd

    if (apResult)
    {
        const std::string name = s_vm->GetTDBIDString(tdbid.value);
        const RED4ext::CString s(name.c_str());
        *static_cast<RED4ext::CString*>(apResult) = s;
    }
}

bool LuaVM::HookRunningStateRun(uintptr_t aThis, uintptr_t aApp)
{
    const auto cNow = std::chrono::high_resolution_clock::now();
    const auto cDelta = cNow - s_vm->m_lastframe;
    const auto cSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(cDelta);

    s_vm->Update(cSeconds.count());

    s_vm->m_lastframe = cNow;

    return s_vm->m_realRunningStateRun(aThis, aApp);
}

uintptr_t LuaVM::HookSetLoadingState(uintptr_t aThis, int aState)
{
    static std::once_flag s_initBarrier;

    if (aState == 2)
    {
        std::call_once(s_initBarrier, []
        {
            s_vm->PostInitializeMods();
        });
    }

    return s_vm->m_realSetLoadingState(aThis, aState);
}

bool LuaVM::HookTranslateBytecode(uintptr_t aBinder, uintptr_t aData)
{
    const auto ret = s_vm->m_realTranslateBytecode(aBinder, aData);

    if (ret)
    {
        s_vm->PostInitializeScripting();
    }

    return ret;
}

uint64_t LuaVM::HookTweakDBLoad(uintptr_t aThis, uintptr_t aParam)
{
    const auto ret = s_vm->m_realTweakDBLoad(aThis, aParam);

    s_vm->PostInitializeTweakDB();

    return ret;
}

void LuaVM::Hook(Options& aOptions)
{
    s_vm = this;

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_Log);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookLog, reinterpret_cast<void**>(&m_realLog)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook Log function!");
            else
                Log::Info("Log function hook complete!");
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_LogChannel);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookLogChannel, reinterpret_cast<void**>(&m_realLogChannel)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook LogChannel function!");
            else
                Log::Info("LogChannel function hook complete!");
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_TDBIDConstructorDerive);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTDBIDCtorDerive, reinterpret_cast<void**>(&m_realTDBIDCtorDerive)) !=
                MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook TDBID::ctor[Derive] function!");
            else
                Log::Info("TDBID::ctor[Derive] function hook complete!");
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_ProcessRunningState);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookRunningStateRun, reinterpret_cast<void**>(&m_realRunningStateRun)) !=
                    MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook RunningState::Run function!");
            else
            {
                Log::Info("RunningState::Run function hook complete!");
            }
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_TranslateBytecode);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTranslateBytecode, reinterpret_cast<void**>(&m_realTranslateBytecode)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook ScriptBinder::TranslateBytecode function!");
            else
            {
                Log::Info("ScriptBinder::TranslateBytecode function hook complete!");
            }
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_ToStringDEBUG);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTDBIDToStringDEBUG,
                              reinterpret_cast<void**>(&m_realTDBIDToStringDEBUG)) !=
                    MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook RunningState::Run function!");
            else
            {
                Log::Info("RunningState::Run function hook complete!");
            }
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_TranslateBytecode);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTranslateBytecode, reinterpret_cast<void**>(&m_realTranslateBytecode)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook ScriptBinder::TranslateBytecode function!");
            else
            {
                Log::Info("ScriptBinder::TranslateBytecode function hook complete!");
            }
        }
    }

    {
        RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CScript_TweakDBLoad);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, &HookTweakDBLoad, reinterpret_cast<void**>(&m_realTweakDBLoad)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook TweakDB::Load function!");
            else
            {
                Log::Info("TweakDB::Load function hook complete!");
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
    //            Log::Error("Could not hook SetLoadingState function!");
    //        else
    //        {
    //            Log::Info("SetLoadingState function hook complete!");
    //        }
    //    }
    //}
}
