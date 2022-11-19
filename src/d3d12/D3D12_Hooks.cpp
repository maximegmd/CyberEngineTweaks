#include <stdafx.h>

#include "D3D12.h"

#include <CET.h>
#include <reverse/Addresses.h>
#include <reverse/RenderContext.h>

void D3D12::CRenderNode_Present_InternalPresent(uint32_t* apSwapChainDataId, uint8_t a2, uint32_t a3)
{
    // apSwapChainDataId is never expected to be nullptr or invalid id!
    assert(apSwapChainDataId && *apSwapChainDataId);

    auto& d3d12 = CET::Get().GetD3D12();

    {
        std::lock_guard statePresentLock(d3d12.m_statePresentMutex);

        if (!d3d12.m_shutdown)
        {
            if (!d3d12.m_initialized)
                d3d12.Initialize(*apSwapChainDataId);

            d3d12.Update(*apSwapChainDataId);
        }
    }

    d3d12.m_realInternalPresent(apSwapChainDataId, a2, a3);
}

void D3D12::CRenderGlobal_Resize(uint32_t aWidth, uint32_t aHeight, uint32_t a3, uint8_t a4, uint32_t* apSwapChainDataId)
{
    // apSwapChainDataId is never expected to be nullptr or invalid id!
    assert(apSwapChainDataId && *apSwapChainDataId);

    auto& d3d12 = CET::Get().GetD3D12();

    d3d12.m_realInternalResize(aWidth, aHeight, a3, a4, apSwapChainDataId);

    std::lock_guard statePresentLock(d3d12.m_statePresentMutex);

    if (!d3d12.m_initialized)
        return;

    if (d3d12.m_swapChainDataId != *apSwapChainDataId)
        return;

    // we need valid render context
    auto* pRenderContext = RenderContext::GetInstance();
    if (pRenderContext == nullptr)
        return;

    // if any back buffer is nullptr, don't continue initialization
    auto swapChainData = pRenderContext->pSwapChainData[*apSwapChainDataId - 1];
    for (auto& pBackBuffer : swapChainData.backBuffers)
    {
        if (pBackBuffer == nullptr)
            return;
    }

    auto backBufferDesc = swapChainData.backBuffers[0]->GetDesc();

    std::lock_guard stateGameLock(d3d12.m_stateGameMutex);

    d3d12.m_resolution = ImVec2(static_cast<float>(backBufferDesc.Width), static_cast<float>(backBufferDesc.Height));

    std::lock_guard imguiLock(d3d12.m_imguiMutex);

    ImGui::GetIO().DisplaySize = d3d12.GetResolution();

    d3d12.ReloadFonts();
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
