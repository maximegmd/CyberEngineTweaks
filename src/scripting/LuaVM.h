#pragma once

#include "Scripting.h"

struct UnknownString;

using TSetMousePosition = BOOL(void*, HWND, long, long);
using TTDBIDCtor = TweakDBID*(TweakDBID*, const char*);
using TTDBIDCtorCString = TweakDBID*(TweakDBID*, const RED4ext::CString*);
using TTDBIDCtorDerive = TweakDBID*(const TweakDBID*, TweakDBID*, const char*);
using TTDBIDCtorUnknown = TweakDBID*(TweakDBID*, uint64_t);
using TSomeStringLookup = UnknownString*(const uint64_t*, UnknownString*);

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
    std::string GetTDBDIDDebugString(TweakDBID aDBID);
    std::string GetTDBIDString(uint64_t aDBID);

    void RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& acString);

protected:
    
    void Hook(Options& aOptions);
    void PostInitialize();
    
    static void HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static void HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static TweakDBID* HookTDBIDCtor(TweakDBID* apThis, const char* acpName);
    static TweakDBID* HookTDBIDCtorCString(TweakDBID* apThis, const RED4ext::CString* acpName);
    static TweakDBID* HookTDBIDCtorDerive(TweakDBID* apBase, TweakDBID* apThis, const char* acpName);
    static TweakDBID* HookTDBIDCtorUnknown(TweakDBID* apThis, uint64_t apName);
    static void HookTDBIDToStringDEBUG(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void* apResult, void*);

private:
  
    std::shared_mutex m_tdbidLock{ };
    TiltedPhoques::Map<uint64_t, TDBIDLookupEntry> m_tdbidLookup{ };
    // Used by TweakDB to get the flats associated with a record
    TiltedPhoques::Map<uint64_t, std::set<uint64_t>> m_tdbidDerivedLookup{ };
    
    RED4ext::OpcodeHandlers::Handler_t m_realLog{ nullptr };
    RED4ext::OpcodeHandlers::Handler_t m_realLogChannel{nullptr};
    TTDBIDCtor* m_realTDBIDCtor{ nullptr };
    TTDBIDCtorCString* m_realTDBIDCtorCString{ nullptr };
    TTDBIDCtorDerive* m_realTDBIDCtorDerive{ nullptr };
    TTDBIDCtorUnknown* m_realTDBIDCtorUnknown{ nullptr };
    TSomeStringLookup* m_someStringLookup{ nullptr };
    RED4ext::OpcodeHandlers::Handler_t m_realTDBIDToStringDEBUG{nullptr};

    std::atomic<uint64_t> m_logCount{ 0 };

    Scripting m_scripting;

    bool m_initialized{ false };
    bool m_drawBlocked{ false };
    
    D3D12& m_d3d12;
    size_t m_connectUpdate;
};
