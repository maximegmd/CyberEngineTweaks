#pragma once

#include "reverse/BasicTypes.h"
#include "common/D3D12Downlevel.h"

typedef TweakDBID TDBID;

struct ScriptContext;
struct ScriptStack;
struct UnknownString;

using TResizeBuffersD3D12 = HRESULT(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
using TPresentD3D12 = HRESULT(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags);
using TPresentD3D12Downlevel = HRESULT(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags);
using TCreateCommittedResource = HRESULT(ID3D12Device *pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource);
using TExecuteCommandLists = void(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
using TSetMousePosition = BOOL(void* apThis, HWND Wnd, long X, long Y);
using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0* apThis);
using TScriptCall = void(ScriptContext*, ScriptStack*, void*, void*);
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
		CComPtr<ID3D12CommandAllocator> CommandAllocator;
		CComPtr<ID3D12Resource> BackBuffer;
		CComPtr<ID3D12Resource> MainRenderTargetResource;
		D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptor{ 0 };
	};

	bool ResetD3D12State();
	bool InitializeD3D12(IDXGISwapChain* pSwapChain);
	bool InitializeD3D12Downlevel(ID3D12CommandQueue* pCommandQueue, ID3D12Resource* pSourceTex2D, HWND hWindow);
	bool InitializeImGui(size_t buffersCounts);
	void Render();
	void DrawImgui();

	static HRESULT ResizeBuffersD3D12(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	static HRESULT PresentD3D12(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags);
	static HRESULT PresentD3D12Downlevel(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags);
	static HRESULT CreateCommittedResourceD3D12(ID3D12Device* pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource);
	static void ExecuteCommandListsD3D12(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
	static BOOL SetMousePosition(void* apThis, HWND Wnd, long X, long Y);
	static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);
	static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void HookLog(ScriptContext* apContext, ScriptStack* apStack, void*, void*);
	static void HookLogChannel(ScriptContext* apContext, ScriptStack* apStack, void*, void*);
	static TDBID* HookTDBIDCtor(TDBID* apThis, const char* apName);
	static TDBID* HookTDBIDCtorCString(TDBID* apThis, const RED4ext::CString* apName);
	static TDBID* HookTDBIDCtorDerive(TDBID* apBase, TDBID* apThis, const char* apName);
	static TDBID* HookTDBIDCtorUnknown(TDBID* apThis, uint64_t apName);
	static void HookTDBIDToStringDEBUG(ScriptContext* apContext, ScriptStack* apStack, void*, void*);

	void RegisterTDBIDString(uint64_t value, uint64_t base, const std::string& string);
	std::string GetTDBIDString(uint64_t value);

private:

	Overlay();

	TResizeBuffersD3D12* m_realResizeBuffersD3D12{ nullptr };
	TPresentD3D12* m_realPresentD3D12{ nullptr };
	TPresentD3D12Downlevel* m_realPresentD3D12Downlevel{ nullptr };
	TCreateCommittedResource* m_realCreateCommittedResource{ nullptr };
	TExecuteCommandLists* m_realExecuteCommandLists{ nullptr };

	std::vector<FrameContext> m_frameContexts;
	std::vector<CComPtr<ID3D12Resource>> m_downlevelBackbuffers;
	CComPtr<IDXGISwapChain3> m_pdxgiSwapChain;
	CComPtr<ID3D12Device> m_pd3d12Device;
	CComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescHeap;
	CComPtr<ID3D12DescriptorHeap> m_pd3dSrvDescHeap;
	CComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;
	CComPtr<ID3D12CommandQueue> m_pCommandQueue;
	uint32_t m_downlevelBufferIndex;

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

	UINT m_outWidth{ 0 };
	UINT m_outHeight{ 0 };
	
	std::recursive_mutex m_outputLock;
	std::vector<std::string> m_outputLines;
	bool m_outputShouldScroll{ true };
	bool m_outputScroll{ false };
	bool m_inputClear{ true };
	bool m_disabledGameLog{ true };
	std::recursive_mutex m_tdbidLock;
	std::unordered_map<uint64_t, TDBIDLookupEntry> m_tdbidLookup;

	bool m_initialized{ false };
};
