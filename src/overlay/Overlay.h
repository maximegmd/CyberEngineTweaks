#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <windows.h>
#include <vector>

struct SomeStruct;


using TPresentD3D12 = long(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
using TSetMousePosition = BOOL(void* apThis, HWND Wnd, long X, long Y);
using TClipToCenter = HWND(SomeStruct* apThis);

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
	bool IsEnabled();

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

	static long PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
	static BOOL SetMousePosition(void* apThis, HWND Wnd, long X, long Y);
	static BOOL ClipToCenter(SomeStruct* apThis);
	
private:

	Overlay();
	
	TPresentD3D12* m_realPresentD3D12{ nullptr };
	std::vector<FrameContext> m_frameContexts;
	ID3D12DescriptorHeap* m_pd3dRtvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_pd3dSrvDescHeap;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	TClipToCenter* m_realClipToCenter{nullptr};
	HWND m_hwnd;
	bool m_enabled{ false };
};
