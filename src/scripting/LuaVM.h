#pragma once

#include "reverse/BasicTypes.h"

typedef TweakDBID TDBID;

struct REDScriptContext;
struct ScriptStack;
struct UnknownString;

using TSetMousePosition = BOOL(void*, HWND, long, long);
using TScriptCall = void(REDScriptContext*, ScriptStack*, void*, void*);
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
	static void Initialize();
	static void Shutdown();
	static LuaVM& Get();
	
  bool ExecuteLua(const std::string& command);

	~LuaVM();
	
	void Update(float deltaTime);

protected:
	
	void Hook();
	
	static void HookLog(REDScriptContext* pContext, ScriptStack* pStack, void*, void*);
	static void HookLogChannel(REDScriptContext* pContext, ScriptStack* pStack, void*, void*);
	static TDBID* HookTDBIDCtor(TDBID* pThis, const char* pName);
	static TDBID* HookTDBIDCtorCString(TDBID* pThis, const RED4ext::CString* pName);
	static TDBID* HookTDBIDCtorDerive(TDBID* pBase, TDBID* pThis, const char* pName);
	static TDBID* HookTDBIDCtorUnknown(TDBID* pThis, uint64_t pName);
	static void HookTDBIDToStringDEBUG(REDScriptContext* pContext, ScriptStack* pStack, void*, void*);

	void RegisterTDBIDString(uint64_t value, uint64_t base, const std::string& string);
	std::string GetTDBIDString(uint64_t value);

private:

	LuaVM();
	
	std::recursive_mutex m_tdbidLock;
	std::unordered_map<uint64_t, TDBIDLookupEntry> m_tdbidLookup;
	
	TScriptCall* m_realLog{ nullptr };
	TScriptCall* m_realLogChannel{ nullptr };
	TTDBIDCtor* m_realTDBIDCtor{ nullptr };
	TTDBIDCtorCString* m_realTDBIDCtorCString{ nullptr };
	TTDBIDCtorDerive* m_realTDBIDCtorDerive{ nullptr };
	TTDBIDCtorUnknown* m_realTDBIDCtorUnknown{ nullptr };
	TSomeStringLookup* m_someStringLookup{ nullptr };
	TScriptCall* m_realTDBIDToStringDEBUG{ nullptr };
	
	bool m_initialized{ false };
};
