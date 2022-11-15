#include "CET.h"

#include <stdafx.h>

#include "D3D12.h"
#include "reverse/Addresses.h"
#include "reverse/RenderContext.h"

void D3D12::CRenderNode_Present_InternalPresent(uint32_t* apSwapChainDataId, uint8_t a2, uint32_t a3)
{
    auto& d3d12 = CET::Get().GetD3D12();

    // D3D12 should always be initialized before getting here from CRenderGlobal_Resize hook
    assert(d3d12.m_initialized);
    if (!d3d12.m_initialized)
        d3d12.Initialize(*apSwapChainDataId);

    // something really wrong is going on - dont update!
    if (!d3d12.m_initialized)
        return;

    d3d12.Update();

    d3d12.m_realInternalPresent(apSwapChainDataId, a2, a3);
}

void D3D12::CRenderGlobal_Resize(uint32_t aWidth, uint32_t aHeight, uint32_t a3, uint8_t a4, uint32_t* apSwapChainDataId)
{
    auto& d3d12 = CET::Get().GetD3D12();

    auto* renderContext = RenderContext::GetInstance();
    assert(renderContext);
    if (!renderContext)
        return d3d12.m_realInternalResize(aWidth, aHeight, a3, a4, apSwapChainDataId);

    if (*apSwapChainDataId == 0 || (d3d12.m_initialized && d3d12.m_swapChainDataId != *apSwapChainDataId))
        return d3d12.m_realInternalResize(aWidth, aHeight, a3, a4, apSwapChainDataId);

    // save original back buffer resource pointers, they will change after real resize here
    auto& pSwapChainBackBuffers = renderContext->pSwapChainData[*apSwapChainDataId - 1].backBuffers;
    ID3D12Resource* pOriginalBackBuffers[SwapChainData_BackBufferCount];
    for (size_t i = 0; i < SwapChainData_BackBufferCount; ++i)
        pOriginalBackBuffers[i] = pSwapChainBackBuffers[i].Get();

    // resize back buffers if they need it
    d3d12.m_realInternalResize(aWidth, aHeight, a3, a4, apSwapChainDataId);

    // check if back buffers changed
    auto backBuffersChanged = false;
    for (size_t i = 0; i < SwapChainData_BackBufferCount; ++i)
        backBuffersChanged |= pOriginalBackBuffers[i] != pSwapChainBackBuffers[i].Get();

    // return if back buffers did not change (may happen)
    if (!backBuffersChanged)
        return;

    if (d3d12.m_initialized)
    {
        Log::Info("CRenderGlobal::Resize() called for our swapchain and backbuffers resized, reinitializing D3D12 resources.");

        d3d12.ResetState();
    }

    d3d12.Initialize(*apSwapChainDataId);
}

std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> D3D12::CreateTextureDescriptor()
{
    auto* pRenderContext = RenderContext::GetInstance();
    auto device = pRenderContext->pDevice;

    const UINT handle_increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    static std::atomic descriptor_index = 1;

    const auto index = descriptor_index++;

    if (index >= 200)
        return {{}, {}};

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += handle_increment * index;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart();
    gpuHandle.ptr += handle_increment * index;

    return {cpuHandle, gpuHandle};
}

void D3D12::Hook()
{
    const RED4ext::RelocPtr<void> presentInternal(CyberEngineTweaks::Addresses::CRenderNode_Present_DoInternal);
    const RED4ext::RelocPtr<void> resizeInternal(CyberEngineTweaks::Addresses::CRenderGlobal_Resize);

    if (MH_CreateHook(presentInternal.GetAddr(), reinterpret_cast<void*>(&CRenderNode_Present_InternalPresent),
                      reinterpret_cast<void**>(&m_realInternalPresent)) != MH_OK ||
        MH_EnableHook(presentInternal.GetAddr()) != MH_OK)
        Log::Error("Could not hook CRenderNode_Present_InternalPresent function!");
    else
        Log::Info("CRenderNode_Present_InternalPresent function hook complete!");

    if (MH_CreateHook(resizeInternal.GetAddr(), reinterpret_cast<void*>(&CRenderGlobal_Resize),
                      reinterpret_cast<void**>(&m_realInternalResize)) != MH_OK ||
        MH_EnableHook(resizeInternal.GetAddr()) != MH_OK)
        Log::Error("Could not hook CRenderGlobal_Resize function!");
    else
        Log::Info("CRenderGlobal_Resize function hook complete!");
}

