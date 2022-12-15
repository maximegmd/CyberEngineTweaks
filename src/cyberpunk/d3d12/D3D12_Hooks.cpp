#include "EngineTweaks.h"

#include <stdafx.h>

#include "D3D12.h"
#include "reverse/Addresses.h"
#include "reverse/RenderContext.h"

#include <kiero/kiero.h>

#include <VersionHelpers.h>


void* D3D12::CRenderNode_Present_InternalPresent(int32_t* apDeviceIndex, uint8_t aSomeSync, UINT aSyncInterval)
{
    static std::once_flag s_kieroOnce;

    auto& d3d12 = EngineTweaks::Get().GetD3D12();

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
            std::call_once(
                s_kieroOnce,
                []
                {
                    if (kiero::init() != kiero::Status::Success)
                        Log::Error("Kiero failed!");
                    else
                    {
                        std::string_view d3d12type = kiero::isDownLevelDevice() ? "D3D12on7" : "D3D12";
                        Log::Info("Kiero initialized for {}", d3d12type);

                        EngineTweaks::Get().GetD3D12().Hook();
                    }
                });
        }
    }

    return d3d12.m_realInternalPresent(apDeviceIndex, aSomeSync, aSyncInterval);
}

void* D3D12::CRenderGlobal_Resize(uint32_t aWidth, uint32_t aHeight, uint32_t a3, uint8_t a4, int* apDeviceIndex)
{
    auto& d3d12 = EngineTweaks::Get().GetD3D12();

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
    auto& d3d12 = EngineTweaks::Get().GetD3D12();

    d3d12.ResetState(true, true);

    return d3d12.m_realInternalShutdown(a1, a2, a3, a4);
}

void D3D12::HookGame()
{
    const RelocPtr<void> presentInternal(Game::Addresses::CRenderNode_Present_DoInternal);
    const RelocPtr<void> resizeInternal(Game::Addresses::CRenderGlobal_Resize);
    const RelocPtr<void> shutdownInternal(Game::Addresses::CRenderGlobal_Shutdown);

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
