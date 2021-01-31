#include "CET.h"

#include <stdafx.h>

#include "D3D12.h"

#include <kiero/kiero.h>

HRESULT D3D12::ResizeBuffers(IDXGISwapChain* apSwapChain, UINT aBufferCount, UINT aWidth, UINT aHeight, DXGI_FORMAT aNewFormat, UINT aSwapChainFlags)
{
    auto& d3d12 = CET::Get().GetD3D12();
    
    if (d3d12.m_initialized)
    {
        // NOTE: right now, done in case of any swap chain ResizeBuffers call, which may not be ideal. We have yet to encounter multiple swap chains in use though, so should be safe
        spdlog::info("D3D12::ResizeBuffers() called with initialized D3D12, triggering D3D12::ResetState.");
        d3d12.ResetState();
    }

    return d3d12.m_realResizeBuffersD3D12(apSwapChain, aBufferCount, aWidth, aHeight, aNewFormat, aSwapChainFlags);
}

HRESULT D3D12::Present(IDXGISwapChain* apSwapChain, UINT aSyncInterval, UINT aPresentFlags)
{
    auto& d3d12 = CET::Get().GetD3D12();

    if (d3d12.Initialize(apSwapChain))
        d3d12.Update(); 
    
    return d3d12.m_realPresentD3D12(apSwapChain, aSyncInterval, aPresentFlags);
}

HRESULT D3D12::PresentDownlevel(ID3D12CommandQueueDownlevel* apCommandQueueDownlevel, ID3D12GraphicsCommandList* apOpenCommandList, ID3D12Resource* apSourceTex2D, HWND ahWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS aFlags)
{
    if (CET::Get().GetOptions().PatchDisableWin7Vsync)
        aFlags &= ~D3D12_DOWNLEVEL_PRESENT_FLAG_WAIT_FOR_VBLANK;

    auto& d3d12 = CET::Get().GetD3D12();

    // On Windows 7 there is no swap chain to query the current backbuffer index. Instead do a reverse lookup in the known backbuffer list
    const auto cbegin = d3d12.m_downlevelBackbuffers.size() >= g_numDownlevelBackbuffersRequired
        ? d3d12.m_downlevelBackbuffers.cend() - g_numDownlevelBackbuffersRequired
        : d3d12.m_downlevelBackbuffers.cbegin();
    auto it = std::find(cbegin, d3d12.m_downlevelBackbuffers.cend(), apSourceTex2D);
    if (it == d3d12.m_downlevelBackbuffers.cend())
    {
        if (d3d12.m_initialized)
        {
            // Already initialized - assume the window was resized and reset state
            d3d12.ResetState();
        }

        // Add the buffer to the list
        d3d12.m_downlevelBackbuffers.emplace_back(apSourceTex2D);
        it = d3d12.m_downlevelBackbuffers.cend() - 1;
    }

    // Limit to at most 3 buffers
    const size_t numBackbuffers = std::min<size_t>(d3d12.m_downlevelBackbuffers.size(), g_numDownlevelBackbuffersRequired);
    const size_t skip = d3d12.m_downlevelBackbuffers.size() - numBackbuffers;
    d3d12.m_downlevelBackbuffers.erase(d3d12.m_downlevelBackbuffers.cbegin(), d3d12.m_downlevelBackbuffers.cbegin() + skip);

    // Determine the current buffer index
    d3d12.m_downlevelBufferIndex = static_cast<uint32_t>(std::distance(d3d12.m_downlevelBackbuffers.cbegin() + skip, it));

    if (d3d12.InitializeDownlevel(d3d12.m_pCommandQueue, apSourceTex2D, ahWindow))
        d3d12.Update();

    return d3d12.m_realPresentD3D12Downlevel(apCommandQueueDownlevel, apOpenCommandList, apSourceTex2D, ahWindow, aFlags);
}

HRESULT D3D12::CreateCommittedResource(ID3D12Device* apDevice, const D3D12_HEAP_PROPERTIES* acpHeapProperties, D3D12_HEAP_FLAGS aHeapFlags, const D3D12_RESOURCE_DESC* acpDesc,
    D3D12_RESOURCE_STATES aInitialResourceState, const D3D12_CLEAR_VALUE* acpOptimizedClearValue, const IID* acpRIID, void** appvResource)
{
    auto& d3d12 = CET::Get().GetD3D12();

    // Check if this is a backbuffer resource being created
    bool isBackBuffer = false;
    if (acpHeapProperties != NULL && acpHeapProperties->Type == D3D12_HEAP_TYPE_DEFAULT && aHeapFlags == D3D12_HEAP_FLAG_NONE &&
        acpDesc != NULL && acpDesc->Flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET && aInitialResourceState == D3D12_RESOURCE_STATE_COMMON &&
        acpOptimizedClearValue == NULL && acpRIID != NULL && IsEqualGUID(*acpRIID, __uuidof(ID3D12Resource)))
    {
        isBackBuffer = true;
    }

    HRESULT result = d3d12.m_realCreateCommittedResource(apDevice, acpHeapProperties, aHeapFlags, acpDesc, aInitialResourceState, acpOptimizedClearValue, acpRIID, appvResource);

    if (SUCCEEDED(result) && isBackBuffer)
    {
        // Store the returned resource
        d3d12.m_downlevelBackbuffers.emplace_back(static_cast<ID3D12Resource*>(*appvResource));
        spdlog::debug("D3D12::CreateCommittedResourceD3D12() - found valid backbuffer target at {0}.", *appvResource);

        if (d3d12.m_initialized)
        {
            // Reset state (a resize may have happened), but don't touch the backbuffer list. The downlevel Present hook will take care of this
            d3d12.ResetState(false);
        }
    }

    return result;
}

void D3D12::ExecuteCommandLists(ID3D12CommandQueue* apCommandQueue, UINT aNumCommandLists, ID3D12CommandList* const* apcpCommandLists)
{
    auto& d3d12 = CET::Get().GetD3D12();

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

    d3d12.m_realExecuteCommandLists(apCommandQueue, aNumCommandLists, apcpCommandLists);
}

void D3D12::Hook()
{
    int d3d12FailedHooksCount = 0;
    int d3d12CompleteHooksCount = 0;
    
    std::string_view d3d12type = (kiero::isDownLevelDevice()) ? ("D3D12on7") : ("D3D12");

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
