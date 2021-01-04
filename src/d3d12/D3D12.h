#pragma once

#include "common/D3D12Downlevel.h"

using TResizeBuffersD3D12 = HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
using TPresentD3D12 = HRESULT(IDXGISwapChain*, UINT, UINT);
using TPresentD3D12Downlevel = HRESULT(ID3D12CommandQueueDownlevel*, ID3D12GraphicsCommandList*, ID3D12Resource*, HWND, D3D12_DOWNLEVEL_PRESENT_FLAGS);
using TCreateCommittedResource = HRESULT(ID3D12Device*, const D3D12_HEAP_PROPERTIES*, D3D12_HEAP_FLAGS, const D3D12_RESOURCE_DESC*, D3D12_RESOURCE_STATES, const D3D12_CLEAR_VALUE*, const IID*, void**);
using TExecuteCommandLists = void(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

struct D3D12
{
	static void Initialize();
	static void Shutdown();
	static D3D12& Get();

	~D3D12();
	
	void PassInputToImGui(bool enabled) { m_passInputToImGui = enabled; }
	void CatchInputInImGui(bool enabled) { m_catchInputInImGui = enabled; }
	
	SIZE GetResolution() const { return m_outSize; }

	LRESULT OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool IsInitialized() const { return m_initialized; }

protected:
	
	void Hook();

	struct FrameContext
	{
		CComPtr<ID3D12CommandAllocator> CommandAllocator;
		CComPtr<ID3D12Resource> BackBuffer;
		CComPtr<ID3D12Resource> MainRenderTargetResource;
		D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptor{ 0 };
	};

	bool ResetState();
	bool Initialize(IDXGISwapChain* pSwapChain);
	bool InitializeDownlevel(ID3D12CommandQueue* pCommandQueue, ID3D12Resource* pSourceTex2D, HWND hWindow);
	bool InitializeImGui(size_t buffersCounts);

	void Update();

	static HRESULT ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	static HRESULT Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags);
	static HRESULT PresentDownlevel(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags);
	static HRESULT CreateCommittedResource(ID3D12Device* pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource);
	static void ExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
	
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
	
	SIZE m_outSize{ };
	
	bool m_passInputToImGui{ false };
	bool m_catchInputInImGui{ false };
};
