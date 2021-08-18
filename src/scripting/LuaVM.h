#pragma once

#include "Scripting.h"
#include "reverse/BasicTypes.h"

typedef TweakDBID TDBID;

struct UnknownString;

using TSetMousePosition = BOOL(void*, HWND, long, long);
using TTDBIDCtorDerive = TDBID*(const TDBID*, TDBID*, const char*);
using TRunningStateRun = bool(uintptr_t, uintptr_t);
using TSetLoadingState = uintptr_t(uintptr_t, int);

struct TDBIDLookupEntry
{
    uint64_t base;
    std::string name;
};

struct Image;
struct LuaVM
{
    LuaVM(Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12, Options& aOptions);
    ~LuaVM();

    const TiltedPhoques::Vector<VKBindInfo>& GetBinds() const;
    
    bool ExecuteLua(const std::string& acCommand);
        
    void Update(float aDeltaTime);
    void Draw();
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
    std::string GetTDBDIDDebugString(TDBID aDBID);
    std::string GetTDBIDString(uint64_t aDBID);

    void RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& acString);

    void PostInitialize();

protected:
    
    void Hook(Options& aOptions);

    static void HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static void HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static TDBID* HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* acpName);
    static bool HookRunningStateRun(uintptr_t aThis, uintptr_t aApp);
    static uintptr_t HookSetLoadingState(uintptr_t aThis, int aState);

private:
  
    std::shared_mutex m_tdbidLock{ };
    TiltedPhoques::Map<uint64_t, TDBIDLookupEntry> m_tdbidLookup{ };
    // Used by TweakDB to get the flats associated with a record
    TiltedPhoques::Map<uint64_t, std::set<uint64_t>> m_tdbidDerivedLookup{ };
    
    RED4ext::OpcodeHandlers::Handler_t m_realLog{ nullptr };
    RED4ext::OpcodeHandlers::Handler_t m_realLogChannel{nullptr};
    TTDBIDCtorDerive* m_realTDBIDCtorDerive{ nullptr };
    TRunningStateRun* m_realRunningStateRun{ nullptr };
    TSetLoadingState* m_realSetLoadingState{ nullptr };

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastframe;

    std::atomic<uint64_t> m_logCount{ 0 };

    Scripting m_scripting;

    bool m_initialized{ false };
    bool m_drawBlocked{ false };
    
    D3D12& m_d3d12;
    size_t m_connectUpdate;
};
