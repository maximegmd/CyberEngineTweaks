#pragma once

#include "common/D3D12Downlevel.h"

using TResizeBuffersD3D12 = HRESULT(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
using TPresentD3D12 = HRESULT(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags);
using TPresentD3D12Downlevel = HRESULT(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags);
using TCreateCommittedResource = HRESULT(ID3D12Device *pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource);
using TExecuteCommandLists = void(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

struct Image;
struct D3D12
{
	static void Initialize(Image* apImage);
	static void Shutdown();
	static D3D12& Get();

	~D3D12();
	
	void Hook();
	
	bool PassInputToImGui{ false };
	bool CatchInputInImGui{ false };

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

	void Update(float deltaTime);

	static HRESULT ResizeBuffersD3D12(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	static HRESULT PresentD3D12(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags);
	static HRESULT PresentD3D12Downlevel(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags);
	static HRESULT CreateCommittedResourceD3D12(ID3D12Device* pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource);
	static void ExecuteCommandListsD3D12(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
	static LRESULT APIENTRY WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:

	D3D12();

	TResizeBuffersD3D12* m_realResizeBuffersD3D12{ nullptr };
	TPresentD3D12* m_realPresentD3D12{ nullptr };
	TPresentD3D12Downlevel* m_realPresentD3D12Downlevel{ nullptr };
	TCreateCommittedResource* m_realCreateCommittedResource{ nullptr };
	TExecuteCommandLists* m_realExecuteCommandLists{ nullptr };
	
	bool m_initialized{ false };

	std::vector<FrameContext> m_frameContexts;
	std::vector<CComPtr<ID3D12Resource>> m_downlevelBackbuffers;
	CComPtr<IDXGISwapChain3> m_pdxgiSwapChain;
	CComPtr<ID3D12Device> m_pd3d12Device;
	CComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescHeap;
	CComPtr<ID3D12DescriptorHeap> m_pd3dSrvDescHeap;
	CComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;
	CComPtr<ID3D12CommandQueue> m_pCommandQueue;
	uint32_t m_downlevelBufferIndex;

	HWND m_hWnd{ nullptr };
	WNDPROC	m_wndProc{ nullptr };

	UINT m_outWidth{ 0 };
	UINT m_outHeight{ 0 };
};
