// CET macOS: macOS shims for Cyber Engine Tweaks. See MacShims.hpp. Pattern: let_there_be_flight/macos/MacShims.cpp.

#include "MacShims.hpp"

#include <MinHook.h>

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include "reverse/Addresses.h"
#include "MacAddresses.hpp"

namespace
{
int g_anchor = 0;
std::atomic<bool> g_trapCalled{false};

// Function-local statics: hashes resolve during static initialization of other files, before this file's globals exist.
std::mutex& LogLock()
{
    static std::mutex s_lock;
    return s_lock;
}

std::set<uint32_t>& TrappedHashes()
{
    static std::set<uint32_t> s_set;
    return s_set;
}

__attribute__((noinline)) uintptr_t cet_mac_trap_stub()
{
    if (!g_trapCalled.exchange(true))
    {
        CetMac::Log("[cet] trap stub called (a function with no Mac address yet); it returns 0. Further calls are not logged.");
    }
    return 0;
}

const char* PluginPathC()
{
    Dl_info info{};
    if (dladdr(&g_anchor, &info) && info.dli_fname)
    {
        return info.dli_fname;
    }
    return nullptr;
}

const RED4ext::v1::Sdk* g_sdk = nullptr;
RED4ext::v1::PluginHandle g_handle = nullptr;
char g_runtimeVersion[32] = "unknown";

struct DeferredHook
{
    uintptr_t Target;
    void* Detour;
    void** Original;
    std::string Name;
    const char* Owner; // the owner's chain symbol
};

std::mutex& DeferredLock()
{
    static std::mutex s_lock;
    return s_lock;
}

std::vector<DeferredHook>& Deferred()
{
    static std::vector<DeferredHook> s_list;
    return s_list;
}

std::atomic<bool> g_cetReady{false};

// The chain symbol of the plugin that also hooks this target (cp2077-archive-xl-macos and cp2077-codeware hook-offsets.txt).
const char* SharedTargetOwner(unsigned long aRva)
{
    switch (aRva)
    {
    case 0x37CE05C: return "archivexl_mac_hook_chain_attach"; // TweakDB::LoadOptimized
    case 0xFFFCB8: return "codeware_mac_hook_chain_attach";  // PlayerSystem::OnPlayerMainObjectSpawned
    default: return nullptr;
    }
}
} // namespace

extern "C" __attribute__((visibility("hidden"))) uintptr_t cet_mac_trap_for_hash(uint32_t aHash)
{
    {
        std::scoped_lock lock(LogLock());
        if (TrappedHashes().insert(aHash).second)
        {
            std::fprintf(stderr, "[cet] address hash %u has no Mac row: it resolves to the fail-safe trap stub (see macos/NOT_YET.md)\n",
                         aHash);
            if (spdlog::default_logger_raw())
            {
                spdlog::warn("[cet] address hash {} has no Mac row: fail-safe trap stub (macos/NOT_YET.md)", aHash);
            }
        }
    }
    return reinterpret_cast<uintptr_t>(&cet_mac_trap_stub);
}

namespace CetMac
{
uintptr_t ImageBase()
{
    static const uintptr_t base = []() -> uintptr_t
    {
        const uint32_t count = _dyld_image_count();
        for (uint32_t i = 0; i < count; ++i)
        {
            const auto* header = reinterpret_cast<const mach_header_64*>(_dyld_get_image_header(i));
            if (header && header->filetype == MH_EXECUTE)
            {
                return reinterpret_cast<uintptr_t>(header);
            }
        }
        return reinterpret_cast<uintptr_t>(_dyld_get_image_header(0));
    }();
    return base;
}

uintptr_t ImageSize()
{
    const auto* header = reinterpret_cast<const mach_header_64*>(ImageBase());
    const auto* command = reinterpret_cast<const load_command*>(header + 1);
    uint64_t textVmaddr = 0;
    uint64_t maxEnd = 0;
    for (uint32_t i = 0; i < header->ncmds; ++i)
    {
        if (command->cmd == LC_SEGMENT_64)
        {
            const auto* segment = reinterpret_cast<const segment_command_64*>(command);
            if (std::strcmp(segment->segname, "__TEXT") == 0)
                textVmaddr = segment->vmaddr;
            if (std::strcmp(segment->segname, "__PAGEZERO") != 0 && std::strcmp(segment->segname, "__LINKEDIT") != 0)
                maxEnd = std::max<uint64_t>(maxEnd, segment->vmaddr + segment->vmsize);
        }
        command = reinterpret_cast<const load_command*>(reinterpret_cast<const uint8_t*>(command) + command->cmdsize);
    }
    return maxEnd > textVmaddr ? static_cast<uintptr_t>(maxEnd - textVmaddr) : 0;
}

uintptr_t VerifiedAddress(uintptr_t aRva, const uint32_t* aExpected, size_t aCount, const char* aName)
{
    const uintptr_t address = ImageBase() + aRva;
    if (aRva + aCount * 4 > ImageSize() || std::memcmp(reinterpret_cast<const void*>(address), aExpected, aCount * 4) != 0)
    {
        Log("[cet] %s: the code at rva 0x%lx is not the expected function (another game build?); feature skipped", aName,
            static_cast<unsigned long>(aRva));
        return 0;
    }
    return address;
}

bool IsSharedTarget(uintptr_t aAddress)
{
    return aAddress > ImageBase() && SharedTargetOwner(static_cast<unsigned long>(aAddress - ImageBase())) != nullptr;
}

bool IsTrap(uintptr_t aAddress)
{
    return aAddress == reinterpret_cast<uintptr_t>(&cet_mac_trap_stub) || aAddress == 0;
}

std::filesystem::path PluginDir()
{
    const char* path = PluginPathC();
    return path ? std::filesystem::path(path).parent_path() : std::filesystem::path();
}

std::filesystem::path RootDir()
{
    // <root>/red4ext/plugins/cyber_engine_tweaks/cyber_engine_tweaks.dylib: three levels up from the plugin folder.
    return PluginDir().parent_path().parent_path().parent_path();
}

std::filesystem::path ExePath()
{
    char buffer[4096];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
    {
        std::error_code error;
        auto canonical = std::filesystem::weakly_canonical(buffer, error);
        return error ? std::filesystem::path(buffer) : canonical;
    }
    return {};
}

void Log(const char* aFormat, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, aFormat);
    std::vsnprintf(buffer, sizeof(buffer), aFormat, args);
    va_end(args);
    std::fprintf(stderr, "%s\n", buffer);
    if (spdlog::default_logger_raw())
    {
        spdlog::info("{}", buffer);
    }
}

RED4ext::GameInstance* GameInstance()
{
    using Fn = RED4ext::GameInstance* (*)();
    static std::atomic<Fn> s_fn{nullptr};
    static std::atomic<bool> s_logged{false};
    Fn fn = s_fn.load();
    if (!fn)
    {
        fn = reinterpret_cast<Fn>(dlsym(RTLD_DEFAULT, "codeware_mac_game_instance"));
        if (!fn)
        {
            if (!s_logged.exchange(true))
                Log("[cet] codeware_mac_game_instance not found: Codeware for macOS is needed for game-instance calls (Game.*, GetSingleton)");
            return nullptr;
        }
        s_fn = fn;
    }
    return fn();
}

void SetPlugin(const RED4ext::v1::Sdk* aSdk, RED4ext::v1::PluginHandle aHandle)
{
    g_sdk = aSdk;
    g_handle = aHandle;
}

const RED4ext::v1::Sdk* PluginSdk()
{
    return g_sdk;
}

RED4ext::v1::PluginHandle PluginHandleValue()
{
    return g_handle;
}

void SetRuntimeVersion(uint8_t aMajor, uint16_t aMinor, uint32_t aPatch)
{
    std::snprintf(g_runtimeVersion, sizeof(g_runtimeVersion), "%u.%u.%u", aMajor, aMinor, aPatch);
}

const char* RuntimeVersionString()
{
    return g_runtimeVersion;
}

void SetCetReady(bool aReady)
{
    g_cetReady = aReady;
}

bool CetReady()
{
    return g_cetReady;
}

bool FlushDeferredHooks()
{
    static std::atomic<bool> s_done{false};
    static int s_ticks = 0;
    if (s_done)
        return true;

    std::scoped_lock lock(DeferredLock());
    ++s_ticks;
    auto& list = Deferred();
    using PeerAttach = bool (*)(uintptr_t, void*, void**);
    for (auto it = list.begin(); it != list.end();)
    {
        const auto rva = static_cast<unsigned long>(it->Target - ImageBase());
        auto chain = reinterpret_cast<PeerAttach>(dlsym(RTLD_DEFAULT, it->Owner));
        void* original = nullptr;
        if (chain && chain(it->Target, it->Detour, &original))
        {
            if (it->Original)
                *it->Original = original;
            Log("[cet] hook %s rva=0x%lx CHAINED on %s (tick %d)", it->Name.c_str(), rva, it->Owner, s_ticks);
            it = list.erase(it);
            continue;
        }
        if (!chain)
        {
            // The owner is not installed: nobody else hooks the target.
            original = nullptr;
            const bool ok = g_sdk && g_sdk->hooking && g_sdk->hooking->Attach &&
                            g_sdk->hooking->Attach(g_handle, reinterpret_cast<void*>(it->Target), it->Detour, &original) &&
                            original != reinterpret_cast<void*>(it->Target);
            if (ok && it->Original)
                *it->Original = original;
            Log("[cet] hook %s rva=0x%lx %s (%s is not loaded)", it->Name.c_str(), rva, ok ? "INSTALLED" : "FAILED", it->Owner);
            it = list.erase(it);
            continue;
        }
        if (s_ticks >= 600)
        {
            Log("[cet] hook %s rva=0x%lx REFUSED: %s is loaded but has not hooked the target after %d ticks; the feature is skipped",
                it->Name.c_str(), rva, it->Owner, s_ticks);
            it = list.erase(it);
            continue;
        }
        ++it;
    }
    if (list.empty())
        s_done = true;
    return s_done;
}

const char* HookResultName(HookResult aResult)
{
    switch (aResult)
    {
    case HookResult::Deferred: return "DEFERRED (owned by another plugin; chained at the first game-state tick)";
    case HookResult::Installed: return "INSTALLED";
    case HookResult::Chained: return "CHAINED";
    case HookResult::NotActive: return "NOT ACTIVE (offset is not on the RED4ext allowlist, macos/hook-offsets.txt)";
    case HookResult::Refused: return "REFUSED";
    case HookResult::Failed: return "FAILED";
    }
    return "?";
}

const char* HookName(uintptr_t aTarget)
{
    using namespace CyberEngineTweaks::AddressHashes;
    static const std::pair<uint32_t, const char*> s_names[] = {
        {CBaseInitializationState_OnTick, "CBaseInitializationState_OnTick"},
        {CInitializationState_OnTick, "CInitializationState_OnTick"},
        {CRunningState_OnTick, "CRunningState_OnTick"},
        {CShutdownState_OnTick, "CShutdownState_OnTick"},
        {CScript_RunPureScript, "CScript_RunPureScript"},
        {CScript_AllocateFunction, "CScript_AllocateFunction"},
        {CScript_Log, "CScript_Log"},
        {CScript_LogError, "CScript_LogError"},
        {CScript_LogWarning, "CScript_LogWarning"},
        {CScript_LogChannel, "CScript_LogChannel"},
        {CScript_LogChannelWarning, "CScript_LogChannelWarning"},
        {CScript_TDBIDConstructorDerive, "CScript_TDBIDConstructorDerive"},
        {CScript_ToStringDEBUG, "CScript_ToStringDEBUG"},
        {CScript_TranslateBytecode, "CScript_TranslateBytecode"},
        {CScript_TweakDBLoad, "CScript_TweakDBLoad"},
        {PlayerSystem_OnPlayerSpawned, "PlayerSystem_OnPlayerSpawned"},
        {CPatches_OptionsInit, "CPatches_OptionsInit"},
    };
    static std::map<uintptr_t, const char*> s_byAddress;
    static std::once_flag s_once;
    std::call_once(s_once,
                   []
                   {
                       for (const auto& [hash, name] : s_names)
                       {
                           const RED4ext::UniversalRelocPtr<uint8_t> ptr(hash);
                           const auto address = reinterpret_cast<uintptr_t>(ptr.GetAddr());
                           if (!IsTrap(address))
                           {
                               s_byAddress.emplace(address, name);
                           }
                       }
                   });
    if (aTarget == ImageBase() + Rva::ScriptBinderLoad)
        return "ScriptBinderLoad";
    const auto it = s_byAddress.find(aTarget);
    return it != s_byAddress.end() ? it->second : "unnamed";
}

HookResult AttachHook(uintptr_t aTarget, void* aDetour, void** aOriginal, const char* aName)
{
    if (aOriginal)
    {
        *aOriginal = nullptr;
    }
    if (const char* skip = std::getenv("CET_MAC_NOHOOKS"))
    {
        if (std::strcmp(skip, "all") == 0 || std::strstr(skip, aName) != nullptr)
        {
            Log("[cet] hook %s SKIPPED by CET_MAC_NOHOOKS=%s", aName, skip);
            return HookResult::Refused;
        }
    }
    if (IsTrap(aTarget))
    {
        Log("[cet] hook %s REFUSED: its address hash has no Mac row (fail-safe, feature skipped)", aName);
        return HookResult::Refused;
    }
    const auto rva = static_cast<unsigned long>(aTarget - ImageBase());
    if (const char* owner = SharedTargetOwner(rva))
    {
        std::scoped_lock lock(DeferredLock());
        Deferred().push_back({aTarget, aDetour, aOriginal, aName, owner});
        Log("[cet] hook %s rva=0x%lx %s: %s", aName, rva, HookResultName(HookResult::Deferred), owner);
        return HookResult::Deferred;
    }
    using PeerAttach = bool (*)(uintptr_t, void*, void**);
    for (const char* peer : {"codeware_mac_hook_chain_attach", "archivexl_mac_hook_chain_attach"})
    {
        if (auto fn = reinterpret_cast<PeerAttach>(dlsym(RTLD_DEFAULT, peer)))
        {
            if (fn(aTarget, aDetour, aOriginal))
            {
                Log("[cet] hook %s rva=0x%lx CHAINED on %s", aName, rva, peer);
                return HookResult::Chained;
            }
            if (aOriginal)
            {
                *aOriginal = nullptr;
            }
        }
    }
    if (!g_sdk || !g_sdk->hooking || !g_sdk->hooking->Attach)
    {
        Log("[cet] hook %s rva=0x%lx FAILED: no RED4ext hooking interface", aName, rva);
        return HookResult::Failed;
    }
    // One attempt.
    if (!g_sdk->hooking->Attach(g_handle, reinterpret_cast<void*>(aTarget), aDetour, aOriginal))
    {
        if (aOriginal)
        {
            *aOriginal = nullptr;
        }
        Log("[cet] hook %s rva=0x%lx FAILED (RED4ext refused the attach; skipped)", aName, rva);
        return HookResult::Failed;
    }
    if (aOriginal && *aOriginal == reinterpret_cast<void*>(aTarget))
    {
        // RED4ext-macos reports success for an offset that is not on RED4EXT_GUM_HOOK_OFFSETS and hands back the target itself.
        *aOriginal = nullptr;
        Log("[cet] hook %s rva=0x%lx %s", aName, rva, HookResultName(HookResult::NotActive));
        return HookResult::NotActive;
    }
    Log("[cet] hook %s rva=0x%lx INSTALLED", aName, rva);
    return HookResult::Installed;
}
} // namespace CetMac

// ---------------------------------------------------------------------------------------------------------------------------------
// MinHook front (macos/inc/MinHook.h).

MH_STATUS MH_Initialize()
{
    return MH_OK;
}

MH_STATUS MH_Uninitialize()
{
    return MH_OK;
}

MH_STATUS MH_CreateHook(void* pTarget, void* pDetour, void** ppOriginal)
{
    // As MinHook: *ppOriginal is written only on success. Several CET call sites keep the target address in that variable and still
    // call it when the hook fails, so it must not become null.
    const auto target = reinterpret_cast<uintptr_t>(pTarget);
    void* original = nullptr;
    const bool shared = CetMac::IsSharedTarget(target);
    const auto result = CetMac::AttachHook(target, pDetour, shared ? ppOriginal : &original, CetMac::HookName(target));
    switch (result)
    {
    case CetMac::HookResult::Deferred:
        // written later by FlushDeferredHooks, into the caller's own variable
        return MH_OK;
    case CetMac::HookResult::Installed:
    case CetMac::HookResult::Chained:
        if (ppOriginal)
            *ppOriginal = original;
        return MH_OK;
    case CetMac::HookResult::NotActive: return MH_ERROR_NOT_EXECUTABLE;
    case CetMac::HookResult::Refused: return MH_ERROR_FUNCTION_NOT_FOUND;
    case CetMac::HookResult::Failed: return MH_UNKNOWN;
    }
    return MH_UNKNOWN;
}

MH_STATUS MH_EnableHook(void*)
{
    return MH_OK;
}

MH_STATUS MH_DisableHook(void*)
{
    return MH_OK;
}

MH_STATUS MH_RemoveHook(void*)
{
    return MH_OK;
}
