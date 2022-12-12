#include <stdafx.h>

#include "LuaVM.h"

#include "EngineTweaks.h"
#include <Utils.h>

#include <reverse/RTTILocator.h>
#include <reverse/TweakDB/ResourcesList.h>

namespace
{

LuaVM* s_vm{nullptr};

using TOodleLZ_Decompress =
    size_t (*)(char* in, int insz, char* out, int outsz, int wantsFuzzSafety, int b, int c, void* d, void* e, void* f, void* g, void* workBuffer, size_t workBufferSize, int j);

struct Header
{
    uint32_t m_magic = 0;   // a hash of all types currently supported
    uint32_t m_version = 0; // 1
    uint32_t m_recordsCount = 0;
    uint32_t m_flatsCount = 0;
    uint32_t m_queriesCount = 0;
};

int32_t ReadCompressedInt(std::istream& aFile)
{
    int32_t uncompressed = 0;

    int8_t byte;
    aFile.read(reinterpret_cast<char*>(&byte), 1);
    uncompressed |= byte & 0x3F;
    if (byte & 0x40)
    {
        aFile.read(reinterpret_cast<char*>(&byte), 1);
        uncompressed |= (byte & 0x7F) << 6;
        if (byte & 0x80)
        {
            aFile.read(reinterpret_cast<char*>(&byte), 1);
            uncompressed |= (byte & 0x7F) << 13;
            if (byte & 0x80)
            {
                aFile.read(reinterpret_cast<char*>(&byte), 1);
                uncompressed |= (byte & 0x7F) << 20;
                if (byte & 0x80)
                {
                    aFile.read(reinterpret_cast<char*>(&byte), 1);
                    uncompressed |= byte << 27;
                }
            }
        }
    }

    return uncompressed;
}

void ReadTDBIDNameArray(std::istream& aFile, uint32_t aCount, TiltedPhoques::Map<uint64_t, TDBIDLookupEntry>& aOutMap)
{
    for (uint32_t i = 0; i < aCount; ++i)
    {
        const int32_t length = ReadCompressedInt(aFile);
        std::string str;
        str.resize(length);
        aFile.read(str.data(), length);
        aOutMap.try_emplace(TweakDBID(str).value, TDBIDLookupEntry{0, str});
    }
}

bool InitializeTweakDBMetadata(TiltedPhoques::Map<uint64_t, TDBIDLookupEntry>& lookup)
{
    auto hOodleHandle = GetModuleHandle(TEXT("oo2ext_7_win64.dll"));
    if (hOodleHandle == nullptr)
    {
        Log::Error("CDPRTweakDBMetadata::Initalize() - Could not get Oodle access");
        return false;
    }

    auto OodleLZ_Decompress = reinterpret_cast<TOodleLZ_Decompress>(GetProcAddress(hOodleHandle, "OodleLZ_Decompress"));
    if (OodleLZ_Decompress == nullptr)
    {
        Log::Error("CDPRTweakDBMetadata::Initalize() - Could not get OodleLZ_Decompress");
        return false;
    }

    const auto tdbstrPathRAW = GetAbsolutePath(L"tweakdb.str", EngineTweaks::Get().GetPaths().TweakDB(), false, true);
    if (!tdbstrPathRAW.empty())
    {
        try
        {
            std::ifstream file(tdbstrPathRAW, std::ios::binary);
            file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
            Header header;
            file.read(reinterpret_cast<char*>(&header), sizeof(Header));
            assert(header.m_version == 1);
            ReadTDBIDNameArray(file, header.m_recordsCount, lookup);
            ReadTDBIDNameArray(file, header.m_flatsCount, lookup);
            ReadTDBIDNameArray(file, header.m_queriesCount, lookup);
        }
        // this is easier for now
        catch (std::exception&)
        {
            Log::Error("CDPRTweakDBMetadata::Initalize() - Failed to load tweakdb.str!");
            return false;
        }
    }
    else
    {
        const auto tdbstrPath = GetAbsolutePath(L"tweakdbstr.kark", EngineTweaks::Get().GetPaths().TweakDB(), false, true);
        if (tdbstrPath.empty())
        {
            Log::Error("CDPRTweakDBMetadata::Initalize() - Missing tweakdbstr.kark!");
            return false;
        }

        try
        {
            constexpr size_t headerSize = 8;
            auto encodedBytes = ReadWholeBinaryFile(tdbstrPath);

            std::string decodedBytes;
            decodedBytes.resize(reinterpret_cast<uint32_t*>(encodedBytes.data())[1]);

            char workingMemory[0x80000];
            auto size = OodleLZ_Decompress(
                encodedBytes.data() + headerSize, static_cast<int>(encodedBytes.size() - headerSize), decodedBytes.data(), static_cast<int>(decodedBytes.size()), 1, 1, 0, nullptr,
                nullptr, nullptr, nullptr, workingMemory, std::size(workingMemory), 3);

            assert(size == decodedBytes.size());
            if (size != decodedBytes.size())
            {
                Log::Error("CDPRTweakDBMetadata::Initalize() - Failed to decompress tweakdbstr.kark!");
                return false;
            }

            struct inline_buffer : std::streambuf
            {
                inline_buffer(char* base, std::ptrdiff_t n) { this->setg(base, base, base + n); }
            } tdbstrDecodedBuffer(decodedBytes.data(), decodedBytes.size());
            std::istream tdbstrDecodedFile(&tdbstrDecodedBuffer);
            tdbstrDecodedFile.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

            Header header;
            tdbstrDecodedFile.read(reinterpret_cast<char*>(&header), sizeof(Header));
            assert(header.m_version == 1);
            ReadTDBIDNameArray(tdbstrDecodedFile, header.m_recordsCount, lookup);
            ReadTDBIDNameArray(tdbstrDecodedFile, header.m_flatsCount, lookup);
            ReadTDBIDNameArray(tdbstrDecodedFile, header.m_queriesCount, lookup);

            Log::Info("CDPRTweakDBMetadata::Initalize() - Primary TweakDB initialization successful!");
        }
        // this is easier for now
        catch (std::exception&)
        {
            Log::Error("CDPRTweakDBMetadata::Initalize() - Failed to load tweakdbstr.kark!");
            return false;
        }
    }

    // check if we have a tweakdb that was changed by REDmod
    const auto moddedTweakDbFilePath = GetAbsolutePath(L"tweakdb.str", EngineTweaks::Get().GetPaths().R6CacheModdedRoot(), false, true);
    if (moddedTweakDbFilePath.empty())
        return true;

    try
    {
        std::ifstream file(moddedTweakDbFilePath, std::ios::binary);
        file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

        Header header;
        file.read(reinterpret_cast<char*>(&header), sizeof(Header));
        assert(header.m_version == 1);
        ReadTDBIDNameArray(file, header.m_recordsCount, lookup);
        ReadTDBIDNameArray(file, header.m_flatsCount, lookup);
        ReadTDBIDNameArray(file, header.m_queriesCount, lookup);
        Log::Info("CDPRTweakDBMetadata::Initalize() - REDMod TweakDB initialization successful!");
    }
    // this is easier for now
    // do not modify the return state since this isn't the main TweakDB
    catch (std::exception&)
    {
        Log::Warn("CDPRTweakDBMetadata::Initalize() - Failed to load REDMod TweakDB. Modded entries may not "
                  "be shown in the TweakDB Editor.");
    }

    return true;
}

} // namespace

void LuaVM::HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    static RTTILocator s_stringLocator("String");

    RED4ext::CString text{};

    apStack->data = nullptr;
    apStack->dataType = nullptr;

    RED4ext::ScriptRef<RED4ext::CString> ref;
    ref.innerType = s_stringLocator;
    ref.ref = &text;
    ref.hash = ref.innerType->GetName();

    apStack->currentParam++;
    const auto opcode = *apStack->code++;

    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &ref, &ref);
    apStack->code++; // skip ParamEnd

    spdlog::get("gamelog")->info("{}", ref.ref->c_str());
}

void LuaVM::HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*)
{
    static RTTILocator s_stringLocator("String");
    static RED4ext::CName s_debugChannel("DEBUG");
    static RED4ext::CName s_assertionChannel("ASSERT");

    RED4ext::CName channel;
    apStack->data = nullptr;
    apStack->dataType = nullptr;
    uint8_t opcode = *apStack->code++;
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
    opcode = *apStack->code++;
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &ref, &ref);

    apStack->code++; // skip ParamEnd

    if (channel == s_debugChannel)
        spdlog::get("scripting")->info("{}", ref.ref->c_str());

    std::string_view channelSV = channel.ToString();
    if (channelSV.empty())
        spdlog::get("gamelog")->info("[?{:X}] {}", channel.hash, ref.ref->c_str());
    else
    {
        if (channel == s_debugChannel)
            spdlog::get("gamelog")->debug("[{}] {}", channelSV, ref.ref->c_str());
        else if (channel == s_assertionChannel)
            spdlog::get("gamelog")->error("[{}] {}", channelSV, ref.ref->c_str());
        else
            spdlog::get("gamelog")->info("[{}] {}", channelSV, ref.ref->c_str());
    }
}

LuaVM::LuaVM(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12)
    : m_scripting(aPaths, aBindings, aD3D12)
    , m_d3d12(aD3D12)
{
    Hook();

    // launch initialization of TweakDBID lookup and resource list on separate thread to not hog the game
    // resource list is currently only used by TweakDB Editor widget, it checks for initialization of this
    // before trying to access it or TweakDBID lookup
    // TweakDBID lookup is on top locked for the duration of the initialization so accessing it should not
    // be possible until it finishes
    std::thread(
        [this]
        {
            {
                std::lock_guard _{m_tdbidLock};
                InitializeTweakDBMetadata(m_tdbidLookup);
            }

            ResourcesList::Get()->Initialize();
        })
        .detach();

    aBindings.SetVM(this);
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
    apStack->data = nullptr;
    apStack->dataType = nullptr;
    apStack->currentParam++;
    const uint8_t opcode = *apStack->code++;
    RED4ext::OpcodeHandlers::Run(opcode, apStack->context, apStack, &tdbid, nullptr);
    apStack->code++; // skip ParamEnd

    if (apResult)
    {
        const std::string name = s_vm->GetTDBIDString(tdbid.value);
        const RED4ext::CString s(name.c_str());
        *static_cast<RED4ext::CString*>(apResult) = s;
    }
}

uintptr_t LuaVM::HookSetLoadingState(uintptr_t aThis, int aState)
{
    static std::once_flag s_initBarrier;

    if (aState == 2)
    {
        std::call_once(s_initBarrier, [] { s_vm->PostInitializeMods(); });
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

uint64_t LuaVM::HookPlayerSpawned(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
    const auto ret = s_vm->m_realPlayerSpawned(a1, a2, a3, a4);

    if (!s_vm->m_initialized)
        s_vm->PostInitializeMods();

    return ret;
}

uint64_t LuaVM::HookTweakDBLoad(uintptr_t aThis, uintptr_t aParam)
{
    const auto ret = s_vm->m_realTweakDBLoad(aThis, aParam);

    s_vm->PostInitializeTweakDB();

    return ret;
}

void LuaVM::Hook()
{
    s_vm = this;

    {
        const RelocPtr<uint8_t> func(Game::Addresses::CScript_Log);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<void*>(&HookLog), reinterpret_cast<void**>(&m_realLog)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook CScript::Log function!");
            else
                Log::Info("CScript::Log function hook complete!");
        }
    }

    {
        const RelocPtr<uint8_t> func(Game::Addresses::CScript_LogChannel);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<void*>(&HookLogChannel), reinterpret_cast<void**>(&m_realLogChannel)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook CScript::LogChannel function!");
            else
                Log::Info("CScript::LogChannel function hook complete!");
        }
    }

    {
        const RelocPtr<uint8_t> func(Game::Addresses::CScript_TDBIDConstructorDerive);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<void*>(&HookTDBIDCtorDerive), reinterpret_cast<void**>(&m_realTDBIDCtorDerive)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook CScript::TDBIDConstructorDerive function!");
            else
                Log::Info("CScript::TDBIDConstructorDerive function hook complete!");
        }
    }

    {
        const RelocPtr<uint8_t> func(Game::Addresses::CScript_ToStringDEBUG);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<LPVOID>(HookTDBIDToStringDEBUG), reinterpret_cast<void**>(&m_realTDBIDToStringDEBUG)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook CScript::ToStringDEBUG function!");
            else
            {
                Log::Info("CScript::ToStringDEBUG function hook complete!");
            }
        }
    }

    {
        const RelocPtr<uint8_t> func(Game::Addresses::CScript_TranslateBytecode);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<LPVOID>(HookTranslateBytecode), reinterpret_cast<void**>(&m_realTranslateBytecode)) != MH_OK ||
                MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook CScript::TranslateBytecode function!");
            else
            {
                Log::Info("CScript::TranslateBytecode function hook complete!");
            }
        }
    }

    {
        const RelocPtr<uint8_t> func(Game::Addresses::CScript_TweakDBLoad);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<LPVOID>(HookTweakDBLoad), reinterpret_cast<void**>(&m_realTweakDBLoad)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook CScript::TweakDBLoad function!");
            else
            {
                Log::Info("CScript::TweakDBLoad function hook complete!");
            }
        }
    }

    {
        const RelocPtr<uint8_t> func(Game::Addresses::PlayerSystem_OnPlayerSpawned);
        uint8_t* pLocation = func.GetAddr();

        if (pLocation)
        {
            if (MH_CreateHook(pLocation, reinterpret_cast<LPVOID>(HookPlayerSpawned), reinterpret_cast<void**>(&m_realPlayerSpawned)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
                Log::Error("Could not hook PlayerSystem::OnPlayerSpawned function!");
            else
            {
                Log::Info("PlayerSystem::OnPlayerSpawned function hook complete!");
            }
        }
    }

    GameMainThread::Get().AddRunningTask(
        [this]
        {
            const auto cNow = std::chrono::high_resolution_clock::now();
            const auto cDelta = cNow - s_vm->m_lastframe;
            const auto cSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(cDelta);

            s_vm->Update(cSeconds.count());

            s_vm->m_lastframe = cNow;

            return false;
        });

    GameMainThread::Get().AddShutdownTask(
        [this]
        {
            s_vm->m_scripting.UnloadAllMods();

            return true;
        });
}
