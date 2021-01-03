#pragma once

#include "reverse/BasicTypes.h"
#include "d3d12/D3D12.h"

typedef TweakDBID TDBID;

struct REDScriptContext;
struct ScriptStack;
struct UnknownString;

using TSetMousePosition = BOOL(void* apThis, HWND Wnd, long X, long Y);
using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0* apThis);
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
struct Console
{
	static void Initialize(Image* apImage);
	static void Shutdown();
	static Console& Get();

	~Console();

	void Toggle();
	bool IsEnabled() const;

	void Log(const std::string& acpText);
	
	void Render();

	LRESULT OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	
	void Hook(Image* apImage);
	void DrawImgui();
	
	static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);
	static void HookLog(REDScriptContext* apContext, ScriptStack* apStack, void*, void*);
	static void HookLogChannel(REDScriptContext* apContext, ScriptStack* apStack, void*, void*);
	static TDBID* HookTDBIDCtor(TDBID* apThis, const char* apName);
	static TDBID* HookTDBIDCtorCString(TDBID* apThis, const RED4ext::CString* apName);
	static TDBID* HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* apName);
	static TDBID* HookTDBIDCtorUnknown(TDBID* apThis, uint64_t apName);
	static void HookTDBIDToStringDEBUG(REDScriptContext* apContext, ScriptStack* apStack, void*, void*);

	void RegisterTDBIDString(uint64_t value, uint64_t base, const std::string& string);
	std::string GetTDBIDString(uint64_t value);

private:

	Console();

	TResizeBuffersD3D12* m_realResizeBuffersD3D12{ nullptr };
	TPresentD3D12* m_realPresentD3D12{ nullptr };
	TPresentD3D12Downlevel* m_realPresentD3D12Downlevel{ nullptr };
	TCreateCommittedResource* m_realCreateCommittedResource{ nullptr };
	TExecuteCommandLists* m_realExecuteCommandLists{ nullptr };
	
	std::recursive_mutex m_outputLock;
	std::vector<std::string> m_outputLines;
	bool m_outputShouldScroll{ true };
	bool m_outputScroll{ false };
	bool m_inputClear{ true };
	bool m_disabledGameLog{ true };
	std::atomic<uint64_t> m_logCount{ 0 };
	
	std::recursive_mutex m_tdbidLock;
	std::unordered_map<uint64_t, TDBIDLookupEntry> m_tdbidLookup;
	
	TClipToCenter* m_realClipToCenter{ nullptr };
	TScriptCall* m_realLog{ nullptr };
	TScriptCall* m_realLogChannel{ nullptr };
	TTDBIDCtor* m_realTDBIDCtor{ nullptr };
	TTDBIDCtorCString* m_realTDBIDCtorCString{ nullptr };
	TTDBIDCtorDerive* m_realTDBIDCtorDerive{ nullptr };
	TTDBIDCtorUnknown* m_realTDBIDCtorUnknown{ nullptr };
	TSomeStringLookup* m_someStringLookup{ nullptr };
	TScriptCall* m_realTDBIDToStringDEBUG{ nullptr };

	HWND m_hWnd{ nullptr };
	WNDPROC	m_wndProc{ nullptr };
	bool m_enabled{ false };
	bool m_focusConsoleInput{ false };
};
