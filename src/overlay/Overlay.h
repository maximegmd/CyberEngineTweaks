#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <windows.h>

using TExecuteCommandListsD3D12 = void(ID3D12CommandQueue*, UINT, ID3D12CommandList**);
using TSignalD3D12 = HRESULT(ID3D12CommandQueue*, ID3D12Fence*, UINT64);
using TDrawInstancedD3D12 = void(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
using TDrawIndexedInstancedD3D12 = void(ID3D12GraphicsCommandList* dCommandList, UINT IndexCount, UINT InstanceCount, UINT StartIndex, INT BaseVertex);
using TPresentD3D12 = long(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);

struct Overlay
{
	static void Initialize();
	static void Shutdown();

	~Overlay();

protected:

	void Hook();

	static void ExecuteCommandListsD3D12(ID3D12CommandQueue*, UINT, ID3D12CommandList**);
	static HRESULT SignalD3D12(ID3D12CommandQueue*, ID3D12Fence*, UINT64);
	static void DrawInstancedD3D12(ID3D12GraphicsCommandList* dCommandList, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation);
	static void DrawIndexedInstancedD3D12(ID3D12GraphicsCommandList* dCommandList, UINT IndexCount, UINT InstanceCount, UINT StartIndex, INT BaseVertex);
	static long PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
	
private:

	Overlay();
	
	TExecuteCommandListsD3D12* m_realExecuteCommandListsD3D12{ nullptr };
	TSignalD3D12* m_realSignalD3D12{ nullptr };
	TDrawInstancedD3D12* m_realDrawInstancedD3D12{ nullptr };
	TDrawIndexedInstancedD3D12* m_realDrawIndexedInstancedD3D12{ nullptr };
	TPresentD3D12* m_realPresentD3D12{ nullptr };
};
