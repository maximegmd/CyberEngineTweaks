#include <stdafx.h>

#include "D3D12.h"

#include <Image.h>
#include <Pattern.h>
#include <kiero/kiero.h>

HRESULT D3D12::ResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    auto& d3d12 = Get();
    
    if (d3d12.m_initialized)
    {
        // NOTE: right now, done in case of any swap chain ResizeBuffers call, which may not be ideal. We have yet to encounter multiple swap chains in use though, so should be safe
        spdlog::info("D3D12::ResizeBuffers() called with initialized D3D12, triggering D3D12::ResetState.");
        d3d12.ResetState();
    }

    return d3d12.m_realResizeBuffersD3D12(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
}

HRESULT D3D12::Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT PresentFlags)
{
    auto& d3d12 = Get();

    if (d3d12.Initialize(pSwapChain))
        d3d12.Update(); 
    
    return d3d12.m_realPresentD3D12(pSwapChain, SyncInterval, PresentFlags);
}

HRESULT D3D12::PresentDownlevel(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags)
{
    auto& d3d12 = Get();

    // On Windows 7 there is no swap chain to query the current backbuffer index, so instead we simply count to 3 and wrap around.
    // Increment the buffer index here even if the d3d12 is not initialized, so we stay in sync with the game's present calls.
    // TODO: investigate if there isn't a better way of doing this (finding the current index in the game exe?)
    d3d12.m_downlevelBufferIndex = (!d3d12.m_initialized || d3d12.m_downlevelBufferIndex == 2) ? 0 : d3d12.m_downlevelBufferIndex + 1;

    if (d3d12.InitializeDownlevel(d3d12.m_pCommandQueue, pSourceTex2D, hWindow))
        d3d12.Update();

    return d3d12.m_realPresentD3D12Downlevel(pCommandQueueDownlevel, pOpenCommandList, pSourceTex2D, hWindow, Flags);
}

HRESULT D3D12::CreateCommittedResource(ID3D12Device* pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource)
{
    auto& d3d12 = Get();

    // Check if this is a backbuffer resource being created
    bool isBackBuffer = false;
    if (pHeapProperties != NULL && pHeapProperties->Type == D3D12_HEAP_TYPE_DEFAULT && HeapFlags == D3D12_HEAP_FLAG_NONE &&
        pDesc != NULL && pDesc->Flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET && InitialResourceState == D3D12_RESOURCE_STATE_COMMON &&
        pOptimizedClearValue == NULL && riidResource != NULL && IsEqualGUID(*riidResource, __uuidof(ID3D12Resource)))
    {
        isBackBuffer = true;
    }

    HRESULT result = d3d12.m_realCreateCommittedResource(pDevice, pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);

    if (SUCCEEDED(result) && isBackBuffer)
    {
        // Store the returned resource
        d3d12.m_downlevelBackbuffers.emplace_back(static_cast<ID3D12Resource*>(*ppvResource));
        spdlog::debug("D3D12::CreateCommittedResourceD3D12() - found valid backbuffer target at {0}.", *ppvResource);
    }

    // If D3D12 has been initialized, there is no need to continue hooking this function since the backbuffers are only created once.
    if (d3d12.m_initialized)
        kiero::unbind(27);

    return result;
}

void D3D12::ExecuteCommandLists(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    auto& d3d12 = Get();

    if (d3d12.m_pCommandQueue == nullptr)
    {
        auto desc = apCommandQueue->GetDesc();
        if(desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) 
        {
            d3d12.m_pCommandQueue = apCommandQueue;
            spdlog::info("D3D12::ExecuteCommandListsD3D12() - found valid command queue.");
        }
        else 
            spdlog::info("D3D12::ExecuteCommandListsD3D12() - ignoring command queue - unusable command list type");
    }

    d3d12.m_realExecuteCommandLists(apCommandQueue, NumCommandLists, ppCommandLists);
}

void D3D12::Hook()
{
    int d3d12FailedHooksCount = 0;
    int d3d12CompleteHooksCount = 0;
    
    const char* d3d12type = (kiero::isDownLevelDevice()) ? ("D3D12on7") : ("D3D12");

    if (kiero::isDownLevelDevice()) 
    {
        if (kiero::bind(175, reinterpret_cast<void**>(&m_realPresentD3D12Downlevel), &PresentDownlevel) != kiero::Status::Success)
        {
            spdlog::error("{0} Downlevel Present hook failed!", d3d12type);
            ++d3d12FailedHooksCount;
        }
        else 
        {
            spdlog::info("{0} Downlevel Present hook complete.", d3d12type);
            ++d3d12CompleteHooksCount;
        }

        if (kiero::bind(27, reinterpret_cast<void**>(&m_realCreateCommittedResource), &CreateCommittedResource) != kiero::Status::Success)
        {
            spdlog::error("{0} CreateCommittedResource Hook failed!", d3d12type);
            ++d3d12FailedHooksCount;
        }
        else 
        {
            spdlog::info("{0} CreateCommittedResource hook complete.", d3d12type);
            ++d3d12CompleteHooksCount;
        }
    }
    else
    {
        if (kiero::bind(140, reinterpret_cast<void**>(&m_realPresentD3D12), &Present) != kiero::Status::Success)
        {
            spdlog::error("{0} Present hook failed!", d3d12type);
            ++d3d12FailedHooksCount;
        }
        else 
        {
            spdlog::info("{0} Present hook complete.", d3d12type);
            ++d3d12CompleteHooksCount;
        }

        if (kiero::bind(145, reinterpret_cast<void**>(&m_realResizeBuffersD3D12), &ResizeBuffers) != kiero::Status::Success)
        {
            spdlog::error("{0} ResizeBuffers hook failed!", d3d12type);
            ++d3d12FailedHooksCount;
        }
        else 
        {
            spdlog::info("{0} ResizeBuffers hook complete.", d3d12type);
            ++d3d12CompleteHooksCount;
        }
    }

    if (kiero::bind(54, reinterpret_cast<void**>(&m_realExecuteCommandLists), &ExecuteCommandLists) != kiero::Status::Success)
    {
        spdlog::error("{0} ExecuteCommandLists hook failed!", d3d12type);
        ++d3d12FailedHooksCount;
    }
    else 
    {
        spdlog::info("{0} ExecuteCommandLists hook complete.", d3d12type);
        ++d3d12CompleteHooksCount;
    }

    if (d3d12FailedHooksCount == 0) 
        spdlog::info("{0} hook complete. ({1}/{2})", d3d12type, d3d12CompleteHooksCount, d3d12CompleteHooksCount+d3d12FailedHooksCount);
    else 
        spdlog::error("{0} hook failed! ({1}/{2})", d3d12type, d3d12CompleteHooksCount, d3d12CompleteHooksCount+d3d12FailedHooksCount);
}
