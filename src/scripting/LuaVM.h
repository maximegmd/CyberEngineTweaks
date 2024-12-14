#pragma once

#include "Scripting.h"
#include "reverse/BasicTypes.h"

typedef TweakDBID TDBID;

struct UnknownString;

using TSetMousePosition = BOOL(void*, HWND, long, long);
using TTDBIDCtorDerive = TDBID*(const TDBID*, TDBID*, const char*);
using TRunningStateRun = bool(uintptr_t, uintptr_t);
using TShutdownStateRun = bool(uintptr_t, uintptr_t);
using TSetLoadingState = uintptr_t(uintptr_t, int);
using TTweakDBLoad = uint64_t(uintptr_t, uintptr_t);
using TTranslateBytecode = void(uintptr_t, uintptr_t);
using TPlayerSpawned = uint64_t(uint64_t, uint64_t, uint64_t, uint64_t);

struct TDBIDLookupEntry
{
    uint64_t base;
    std::string name;
};

struct Image;
struct LuaVM
{
    LuaVM(const Paths& aPaths, const Options& aOptions, VKBindings& aBindings, D3D12& aD3D12);
    ~LuaVM() = default;

    [[nodiscard]] const VKBind* GetBind(const VKModBind& acModBind) const;
    [[nodiscard]] const TiltedPhoques::Vector<VKBind>* GetBinds(const std::string& acModName) const;
    [[nodiscard]] const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& GetAllBinds() const;

    void Prepare();

    bool ExecuteLua(const std::string& acCommand) const;
    int ThrowLuaError(const std::string& acMessage) const;

    void Update(float aDeltaTime);
    void Draw() const;
    void ReloadAllMods();

    void OnOverlayOpen() const;
    void OnOverlayClose() const;

    void Initialize();

    bool IsInitialized() const;

    void BlockDraw(bool aBlockDraw);

    // Used by TweakDB when you delete a custom record
    void RemoveTDBIDDerivedFrom(uint64_t aDBID);
    bool GetTDBIDDerivedFrom(uint64_t aDBID, TiltedPhoques::Vector<uint64_t>& aDerivedList);
    uint64_t GetTDBIDBase(uint64_t aDBID);
    TDBIDLookupEntry GetTDBIDLookupEntry(uint64_t aDBID);
    std::string GetTDBDIDDebugString(TDBID aDBID) const;
    std::string GetTDBIDString(uint64_t aDBID, bool aOnlyRegistered = false);

    void RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& acString);

    void PostInitializeScripting();
    void PostInitializeTweakDB();
    void PostInitializeMods();

protected:
    void Hook();

    static void HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static void HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static void HookTDBIDToStringDEBUG(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void* apResult, void*);
    static TDBID* HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* acpName);
    static uintptr_t HookSetLoadingState(uintptr_t aThis, int aState);
    static uint64_t HookTweakDBLoad(uintptr_t aThis, uintptr_t aParam);
    static void HookTranslateBytecode(uintptr_t aBinder, uintptr_t aData);
    static uint64_t HookPlayerSpawned(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4);

private:
    std::mutex m_tdbidLock{};
    TiltedPhoques::Map<uint64_t, TDBIDLookupEntry> m_tdbidLookup{};
    // Used by TweakDB to get the flats associated with a record
    TiltedPhoques::Map<uint64_t, std::set<uint64_t>> m_tdbidDerivedLookup{};

    RED4ext::OpcodeHandlers::Handler_t m_realLog{nullptr};
    RED4ext::OpcodeHandlers::Handler_t m_realLogError{nullptr};
    RED4ext::OpcodeHandlers::Handler_t m_realLogWarning{nullptr};
    RED4ext::OpcodeHandlers::Handler_t m_realLogChannel{nullptr};
    RED4ext::OpcodeHandlers::Handler_t m_realLogChannelWarning{nullptr};
    RED4ext::OpcodeHandlers::Handler_t m_realTDBIDToStringDEBUG{nullptr};
    TTDBIDCtorDerive* m_realTDBIDCtorDerive{nullptr};
    TSetLoadingState* m_realSetLoadingState{nullptr};
    TTweakDBLoad* m_realTweakDBLoad{nullptr};
    TTranslateBytecode* m_realTranslateBytecode{nullptr};
    TPlayerSpawned* m_realPlayerSpawned{nullptr};

    Scripting m_scripting;

    bool m_initialized{false};
    bool m_drawBlocked{false};
    bool m_reload{false};

    D3D12& m_d3d12;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastframe{std::chrono::high_resolution_clock::now()};
};
