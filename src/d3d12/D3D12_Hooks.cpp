#include "CET.h"

#include <stdafx.h>

#include "D3D12.h"
#include "reverse/Addresses.h"
#include "reverse/RenderContext.h"

#include <VersionHelpers.h>

void* ApplyHook(void** vtable, size_t index, void* target)
{
    DWORD oldProtect;
    VirtualProtect(vtable + index, 8, PAGE_EXECUTE_READWRITE, &oldProtect);
    const auto ret = vtable[index];
    vtable[index] = target;
    VirtualProtect(vtable + index, 8, oldProtect, nullptr);

    return ret;
}

void* D3D12::CRenderNode_Present_InternalPresent(int32_t* apDeviceIndex, uint8_t aSomeSync, UINT aSyncInterval)
{
    auto& d3d12 = CET::Get().GetD3D12();

    const auto* pContext = RenderContext::GetInstance();
    auto* pSwapChain = pContext->devices[*apDeviceIndex - 1].pSwapChain;
    if (d3d12.m_initialized)
        d3d12.Update();
    else
    {
        // NOTE: checking against Windows 8 as Windows 10 requires specific compatibility manifest to be detected by
        // these
        //       DX12 does not work on Windows 8 and 8.1 so we should be safe with this check
        if (IsWindows8OrGreater())
        {
            d3d12.m_pCommandQueue = pContext->pDirectQueue;
            d3d12.m_pdxgiSwapChain = pSwapChain;
            d3d12.Initialize();
        }
        else
        {
            Log::Error("Unsupported OS!");
        }
    }

    return d3d12.m_realInternalPresent(apDeviceIndex, aSomeSync, aSyncInterval);
}

void* D3D12::CRenderGlobal_Resize(uint32_t aWidth, uint32_t aHeight, uint32_t a3, uint8_t a4, int* apDeviceIndex)
{
    auto& d3d12 = CET::Get().GetD3D12();

    // TODO - ideally find a way to not call this on each minimize/maximize/etc. which causes this to be called
    //        it can get called multiple times even when there was no resolution change or swapchain invalidation
    if (d3d12.m_initialized)
    {
        Log::Info("CRenderGlobal::Resize() called with initialized D3D12, triggering D3D12::ResetState.");
        d3d12.ResetState();
    }

    return d3d12.m_realInternalResize(aWidth, aHeight, a3, a4, apDeviceIndex);
}

// NOTE - this is called 32 times, as it seems to be called for each device object in RendererContext
void* D3D12::CRenderGlobal_Shutdown(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
    auto& d3d12 = CET::Get().GetD3D12();

    d3d12.ResetState(true);

    return d3d12.m_realInternalShutdown(a1, a2, a3, a4);
}

ID3D12Device* D3D12::GetDevice() const
{
    return m_pd3d12Device.Get();
}

std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> D3D12::CreateTextureDescriptor()
{
    const UINT handle_increment = m_pd3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
    const RED4ext::UniversalRelocPtr<void> presentInternal(CyberEngineTweaks::AddressHashes::CRenderNode_Present_DoInternal);
    const RED4ext::UniversalRelocPtr<void> resizeInternal(CyberEngineTweaks::AddressHashes::CRenderGlobal_Resize);
    const RED4ext::UniversalRelocPtr<void> shutdownInternal(CyberEngineTweaks::AddressHashes::CRenderGlobal_Shutdown);

    if (MH_CreateHook(presentInternal.GetAddr(), reinterpret_cast<void*>(&CRenderNode_Present_InternalPresent), reinterpret_cast<void**>(&m_realInternalPresent)) != MH_OK ||
        MH_EnableHook(presentInternal.GetAddr()) != MH_OK)
        Log::Error("Could not hook CRenderNode_Present_InternalPresent function!");
    else
        Log::Info("CRenderNode_Present_InternalPresent function hook complete!");

    if (MH_CreateHook(resizeInternal.GetAddr(), reinterpret_cast<void*>(&CRenderGlobal_Resize), reinterpret_cast<void**>(&m_realInternalResize)) != MH_OK ||
        MH_EnableHook(resizeInternal.GetAddr()) != MH_OK)
        Log::Error("Could not hook CRenderGlobal_Resize function!");
    else
        Log::Info("CRenderGlobal_Resize function hook complete!");

    if (MH_CreateHook(shutdownInternal.GetAddr(), reinterpret_cast<void*>(&CRenderGlobal_Shutdown), reinterpret_cast<void**>(&m_realInternalShutdown)) != MH_OK ||
        MH_EnableHook(shutdownInternal.GetAddr()) != MH_OK)
        Log::Error("Could not hook CRenderGlobal_Shutdown function!");
    else
        Log::Info("CRenderGlobal_Shutdown function hook complete!");
}
