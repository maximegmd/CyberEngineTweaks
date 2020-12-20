#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <mutex>
#include <string>
#include <windows.h>
#include <vector>

#include "reverse/Engine.h"

using TPresentD3D12 = long(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
using TSetMousePosition = BOOL(void* apThis, HWND Wnd, long X, long Y);
using TClipToCenter = HWND(CGameEngine::UnkC0* apThis);
using TLog = void*(uintptr_t a1, uint8_t** a2);

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
	static void* HookLog(uintptr_t apThis, uint8_t** apStack);
	
private:

	Overlay();
	
	TPresentD3D12* m_realPresentD3D12{ nullptr };
	std::vector<FrameContext> m_frameContexts;
	ID3D12DescriptorHeap* m_pd3dRtvDescHeap = nullptr;
	ID3D12DescriptorHeap* m_pd3dSrvDescHeap;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	TClipToCenter* m_realClipToCenter{nullptr};
	TLog* m_realLog{nullptr};
	HWND m_hwnd;
	WNDPROC	m_wndProc{nullptr};
	bool m_enabled{ false };
	
	std::recursive_mutex m_outputLock;
	std::vector<std::string> m_outputLines;
	bool m_outputShouldScroll{ true };
	bool m_outputScroll{ false };
	bool m_inputClear{ true };
};
