#pragma once

#include "Scripting.h"
#include "reverse/BasicTypes.h"

typedef TweakDBID TDBID;

struct UnknownString;

using TSetMousePosition = BOOL(void*, HWND, long, long);
using TTDBIDCtor = TDBID*(TDBID*, const char*);
using TTDBIDCtorCString = TDBID*(TDBID*, const RED4ext::CString*);
using TTDBIDCtorDerive = TDBID*(const TDBID*, TDBID*, const char*);
using TTDBIDCtorUnknown = TDBID*(TDBID*, uint64_t);
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

    const std::vector<VKBindInfo>& GetBinds() const;
    
    bool ExecuteLua(const std::string& acCommand);
        
    void Update(float aDeltaTime);
    void ReloadAllMods();

    void OnOverlayOpen() const;
    void OnOverlayClose() const;

    void Initialize();

    bool IsInitialized() const;

protected:
    
    void Hook(Options& aOptions);
    void PostInitialize();
    
    static void HookLog(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static void HookLogChannel(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void*, void*);
    static TDBID* HookTDBIDCtor(TDBID* apThis, const char* acpName);
    static TDBID* HookTDBIDCtorCString(TDBID* apThis, const RED4ext::CString* acpName);
    static TDBID* HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* acpName);
    static TDBID* HookTDBIDCtorUnknown(TDBID* apThis, uint64_t apName);
    static void HookTDBIDToStringDEBUG(RED4ext::IScriptable*, RED4ext::CStackFrame* apStack, void* apResult, void*);

    void RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& acString);
    std::string GetTDBIDString(uint64_t aValue);

private:
  
    std::recursive_mutex m_tdbidLock{ };
    std::unordered_map<uint64_t, TDBIDLookupEntry> m_tdbidLookup{ };
    
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

    D3D12& m_d3d12;
    size_t m_connectUpdate;
};
