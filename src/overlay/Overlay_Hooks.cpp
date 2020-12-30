#include <stdafx.h>

#include "Overlay.h"

#include <Image.h>
#include <Pattern.h>
#include <kiero/kiero.h>

void Overlay::EarlyHooks(Image* apImage)
{
    uint8_t* pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x8B,
        0x99, 0x68, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xF9, 0xFF });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &ClipToCenter, reinterpret_cast<void**>(&m_realClipToCenter)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook mouse clip function!");
        else
            spdlog::info("\tHook mouse clip function!");
    }

    pLocation = FindSignature({
        0x40, 0x53, 0x48, 0x83, 0xEC, 0x40, 0x48, 0x8B,
        0xDA, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B,
        0xD0, 0x48, 0x8D, 0x4C, 0x24, 0x20
    });

    if(pLocation)
    {
        if (MH_CreateHook(pLocation, &HookLog, reinterpret_cast<void**>(&m_realLog)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook Log function!");
        else
            spdlog::info("\tLog function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x40, 0x48,
        0x8B, 0x02, 0x48, 0x8D, 0x3D, 0xCC, 0xCC, 0xCC,
        0xCC, 0x33, 0xF6, 0x4C, 0x8D, 0x44, 0x24, 0x58,
        0x48, 0x89, 0x74, 0x24, 0x58, 0x45, 0x33, 0xC9,
        0x48, 0x89, 0x72, 0x30, 0x48, 0x8B, 0xDA, 0x48,
        0x89, 0x72, 0x38, 0x0F, 0xB6, 0x08, 0x48, 0xFF,
        0xC0, 0x48, 0x89, 0x02, 0x8B, 0xC1, 0x48, 0x8B,
        0x4A, 0x40, 0xFF, 0x14, 0xC7, 0xE8, 0xCC, 0xCC,
        0xCC, 0xCC, 0x48, 0x8B, 0xD0
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookLogChannel, reinterpret_cast<void**>(&m_realLogChannel)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook LogChannel function!");
        else
            spdlog::info("\tLogChannel function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x40, 0x80,
        0x3A, 0x00, 0x48, 0x8B, 0xFA
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtor, reinterpret_cast<void**>(&m_realTDBIDCtor)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook TDBID::ctor function!");
        else
            spdlog::info("\tTDBID::ctor function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48,
        0x8B, 0xF1, 0x48, 0x8B, 0xDA, 0x48, 0x8B, 0xCA,
        0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCB
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtorCString, reinterpret_cast<void**>(&m_realTDBIDCtorCString)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook TDBID::ctor[CString] function!");
        else
            spdlog::info("\tTDBID::ctor[CString] function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74,
        0x24, 0x18, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x33,
        0xC0, 0x4D, 0x8B, 0xC8, 0x48, 0x8B, 0xF2, 0x4D,
        0x85, 0xC0, 0x74, 0x0F, 0x41, 0x38, 0x00,
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtorDerive, reinterpret_cast<void**>(&m_realTDBIDCtorDerive)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook TDBID::ctor[Derive] function!");
        else
            spdlog::info("\tTDBID::ctor[Derive] function hook complete!");
    }

    pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x54,
        0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x50, 0x48,
        0x8B, 0xF9, 0x48, 0x8D, 0x54, 0x24, 0x20, 0x48,
        0x8D, 0x4C, 0x24, 0x68, 0xE8
        });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &HookTDBIDCtorUnknown, reinterpret_cast<void**>(&m_realTDBIDCtorUnknown)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("\tCould not hook TDBID::ctor[Unknown] function!");
        else
        {
            spdlog::info("\tTDBID::ctor[Unknown] function hook complete!");
            *reinterpret_cast<void**>(&m_someStringLookup) = &pLocation[33] + *reinterpret_cast<int32_t*>(&pLocation[29]);
        }
    }

    pLocation = FindSignature({
        0x48, 0xBF, 0x58, 0xD1, 0x78, 0xA0, 0x18, 0x09,
        0xBA, 0xEC, 0x75, 0x16, 0x48, 0x8D, 0x15, 0xCC,
        0xCC, 0xCC, 0xCC, 0x48, 0x8B, 0xCF, 0xE8, 0xCC,
        0xCC, 0xCC, 0xCC, 0xC6, 0x05, 0xCC, 0xCC, 0xCC,
        0xCC, 0x01, 0x41, 0x8B, 0x06, 0x39, 0x05, 0xCC,
        0xCC, 0xCC, 0xCC, 0x7F
        });

    if (pLocation)
    {
        pLocation = &pLocation[45] + static_cast<int8_t>(pLocation[44]);
        pLocation = FindSignature(pLocation, pLocation + 45, {
            0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8,
            0xCC, 0xCC, 0xCC, 0xCC, 0x83, 0x3D, 0xCC, 0xCC,
            0xCC, 0xCC, 0xFF, 0x75, 0xCC, 0x48, 0x8D, 0x05,
            });
        if (pLocation)
        {
            pLocation = &pLocation[28] + *reinterpret_cast<int32_t*>(&pLocation[24]);
            if (MH_CreateHook(pLocation, &HookTDBIDToStringDEBUG, reinterpret_cast<void**>(&m_realTDBIDToStringDEBUG)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
                spdlog::error("\tCould not hook TDBID::ToStringDEBUG function!");
            else
                spdlog::info("\tTDBID::ToStringDEBUG function hook complete!");
        }
    }
}

long Overlay::PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    auto& overlay = Get();

    if (overlay.InitializeD3D12(pSwapChain))
        overlay.Render(pSwapChain);
    
    return overlay.m_realPresentD3D12(pSwapChain, SyncInterval, Flags);
}

HRESULT Overlay::PresentD3D12Downlevel(ID3D12CommandQueueDownlevel* pCommandQueueDownlevel, ID3D12GraphicsCommandList* pOpenCommandList, ID3D12Resource* pSourceTex2D, HWND hWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS Flags)
{
    auto& overlay = Get();

    // On Windows 7 there is no swap chain to query the current backbuffer index, so instead we simply count to 3 and wrap around.
    // Increment the buffer index here even if the overlay is not enabled, so we stay in sync with the game's present calls.
    // TODO: investigate if there isn't a better way of doing this (finding the current index in the game exe?)
    overlay.m_downlevelBufferIndex = (!overlay.m_initialized || overlay.m_downlevelBufferIndex == 2) ? 0 : overlay.m_downlevelBufferIndex + 1;

    if (overlay.InitializeD3D12Downlevel(overlay.m_pCommandQueue, pSourceTex2D))
    {
        overlay.Render(nullptr);
    }

    return overlay.m_realPresentD3D12Downlevel(pCommandQueueDownlevel, pOpenCommandList, pSourceTex2D, hWindow, Flags);
}

HRESULT Overlay::CreateCommittedResourceD3D12(ID3D12Device* pDevice, const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc,
    D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, const IID* riidResource, void** ppvResource)
{
    auto& overlay = Get();

    // Check if this is a backbuffer resource being created
    bool isBackBuffer = false;
    if (pHeapProperties != NULL && pHeapProperties->Type == D3D12_HEAP_TYPE_DEFAULT && HeapFlags == D3D12_HEAP_FLAG_NONE &&
        pDesc != NULL && pDesc->Flags == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET && InitialResourceState == D3D12_RESOURCE_STATE_COMMON &&
        pOptimizedClearValue == NULL && riidResource != NULL && IsEqualGUID(*riidResource, __uuidof(ID3D12Resource)))
    {
        spdlog::debug("\tOverlay::CreateCommittedResourceD3D12() - found valid backbuffer target.");
        isBackBuffer = true;
    }

    HRESULT result = overlay.m_realCreateCommittedResource(pDevice, pHeapProperties, HeapFlags, pDesc, InitialResourceState, pOptimizedClearValue, riidResource, ppvResource);

    if (SUCCEEDED(result) && isBackBuffer)
    {
        // Store the returned resource
        overlay.m_downlevelBackbuffers.emplace_back(static_cast<ID3D12Resource*>(*ppvResource));
    }

    // If D3D12 has been initialized, there is no need to continue hooking this function since the backbuffers are only created once.
    if (overlay.m_initialized)
        kiero::unbind(27);

    return result;
}

void Overlay::ExecuteCommandListsD3D12(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    auto& overlay = Get();
    if (overlay.m_pCommandQueue == nullptr)
    {
        auto desc = apCommandQueue->GetDesc();
        if(desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT) 
        {
            overlay.m_pCommandQueue = apCommandQueue;
            spdlog::info("\tOverlay::ExecuteCommandListsD3D12() - found valid command queue.");
        }
        else 
            spdlog::info("\tOverlay::ExecuteCommandListsD3D12() - ignoring unusable command list type");
    }

    overlay.m_realExecuteCommandLists(apCommandQueue, NumCommandLists, ppCommandLists);
}

BOOL Overlay::ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
{
    const HWND wnd = (HWND)apThis->hWnd;
    const HWND foreground = GetForegroundWindow();

    if(wnd == foreground && apThis->unk164 && !apThis->unk140 && !Get().IsEnabled())
    {
        RECT rect;
        GetClientRect(wnd, &rect);
        ClientToScreen(wnd, reinterpret_cast<POINT*>(&rect.left));
        ClientToScreen(wnd, reinterpret_cast<POINT*>(&rect.right));
        rect.left = (rect.left + rect.right) / 2;
        rect.right = rect.left;
        rect.bottom = (rect.bottom + rect.top) / 2;
        rect.top = rect.bottom;
        apThis->isClipped = true;
        ShowCursor(FALSE);
        return ClipCursor(&rect);
    }

    if(apThis->isClipped)
    {
        apThis->isClipped = false;
        return ClipCursor(nullptr);
    }

    return 1;
}

void Overlay::Hook()
{
    // D3D12 hook
    int d3d12FailedHooksCount = 0;

    if (!kiero::isDownLevelDevice() && kiero::bind(140, reinterpret_cast<void**>(&m_realPresentD3D12), &PresentD3D12) != kiero::Status::Success) 
    {
        spdlog::error("\tD3D12 Present hook failed!");
        ++d3d12FailedHooksCount;
    }
    else if (kiero::isDownLevelDevice())
    {
        if (kiero::bind(135, reinterpret_cast<void**>(&m_realPresentD3D12Downlevel), &PresentD3D12Downlevel) != kiero::Status::Success)
        {
            spdlog::error("\tD3D12On7 Downlevel Present Hook failed!");
            ++d3d12FailedHooksCount;
        }
        if (kiero::bind(27, reinterpret_cast<void**>(&m_realCreateCommittedResource), &CreateCommittedResourceD3D12) != kiero::Status::Success)
        {
            spdlog::error("\tD3D12On7 CreateCommittedResource Hook failed!");
            ++d3d12FailedHooksCount;
        }
    }

    if (kiero::bind(54, reinterpret_cast<void**>(&m_realExecuteCommandLists), &ExecuteCommandListsD3D12) != kiero::Status::Success)
    {
        spdlog::error("\tD3D12 ExecuteCommandLists hook failed!");
        ++d3d12FailedHooksCount;
    }

    if (d3d12FailedHooksCount == 0) 
        spdlog::info("\tD3D12 hook complete.");
    else 
        spdlog::error("\tD3D12 hook failed!");
}
