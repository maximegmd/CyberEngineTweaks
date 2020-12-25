#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <mutex>
#include <string>
#include <windows.h>
#include <unordered_map>
#include <vector>

#include "reverse/BasicTypes.h"
#include "reverse/Engine.h"
#include "RED4ext/REDreverse/CString.hpp"

typedef TweakDBID TDBID;

struct ScriptContext;
struct ScriptStack;
struct UnknownString;

using TPresentD3D12 = long(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
using TExecuteCommandLists = void(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
using TSetMousePosition = BOOL(void* apThis, HWND Wnd, long X, long Y);
using TClipToCenter = HWND(CGameEngine::UnkC0* apThis);
using TScriptCall = void(ScriptContext*, ScriptStack*, void*, void*);
using TTDBIDCtor = TDBID*(TDBID*, const char*);
using TTDBIDCtorCString = TDBID*(TDBID*, const RED4ext::REDreverse::CString*);
using TTDBIDCtorDerive = TDBID*(const TDBID*, TDBID*, const char*);
using TTDBIDCtorUnknown = TDBID*(TDBID*, uint64_t);
using TSomeStringLookup = UnknownString*(const uint64_t*, UnknownString*);

struct TDBIDLookupEntry
{
	uint64_t base;
	std::string name;
};

struct Image;
struct Overlay
{
	static void Initialize(Image* apImage);
	static void Shutdown();
	static Overlay& Get();

	~Overlay();

	void EarlyHooks(Image* apImage);
	void Hook();

	void Toggle();
	bool IsEnabled() const;

	void Log(const std::string& acpText);

protected:

	struct FrameContext
	{
		ID3D12Resource* MainRenderTargetResource = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptor;
		ID3D12Resource* BackBuffer = nullptr;
		ID3D12CommandAllocator* CommandAllocator = nullptr;
	};

	void InitializeD3D12(IDXGISwapChain3* pSwapChain);
	void Render(IDXGISwapChain3* pSwapChain);
	void DrawImgui(IDXGISwapChain3* apSwapChain);

	static long PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
	static BOOL SetMousePosition(void* apThis, HWND Wnd, long X, long Y);
	static BOOL ClipToCenter(CGameEngine::UnkC0* apThis);
	static LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void HookLog(ScriptContext* apContext, ScriptStack* apStack, void*, void*);
	static void HookLogChannel(ScriptContext* apContext, ScriptStack* apStack, void*, void*);
	static TDBID* HookTDBIDCtor(TDBID* apThis, const char* apName);
	static TDBID* HookTDBIDCtorCString(TDBID* apThis, const RED4ext::REDreverse::CString* apName);
	static TDBID* HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* apName);
	static TDBID* HookTDBIDCtorUnknown(TDBID* apThis, uint64_t apName);
	static void HookTDBIDToStringDEBUG(ScriptContext* apContext, ScriptStack* apStack, void*, void*);

	void RegisterTDBIDString(uint64_t value, uint64_t base, const std::string& string);
	std::string GetTDBIDString(uint64_t value);

private:

	Overlay();
	
	TPresentD3D12* m_realPresentD3D12{ nullptr };
	TExecuteCommandLists* m_realExecuteCommandLists{ nullptr };
	std::vector<FrameContext> m_frameContexts;
	ID3D12DescriptorHeap* m_pd3dRtvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_pd3dSrvDescHeap;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	ID3D12CommandQueue* m_pCommandQueue{ nullptr };
	TClipToCenter* m_realClipToCenter{nullptr};
	TScriptCall* m_realLog{nullptr};
	TScriptCall* m_realLogChannel{ nullptr };
	TTDBIDCtor* m_realTDBIDCtor{ nullptr };
	TTDBIDCtorCString* m_realTDBIDCtorCString{ nullptr };
	TTDBIDCtorDerive* m_realTDBIDCtorDerive{ nullptr };
	TTDBIDCtorUnknown* m_realTDBIDCtorUnknown{ nullptr };
	TSomeStringLookup* m_someStringLookup{ nullptr };
	TScriptCall* m_realTDBIDToStringDEBUG{ nullptr };
	HWND m_hwnd;
	WNDPROC	m_wndProc{nullptr};
	bool m_enabled{ false };
	
	std::recursive_mutex m_outputLock;
	std::vector<std::string> m_outputLines;
	bool m_outputShouldScroll{ true };
	bool m_outputScroll{ false };
	bool m_inputClear{ true };
	std::recursive_mutex m_tdbidLock;
	std::unordered_map<uint64_t, TDBIDLookupEntry> m_tdbidLookup;
};
