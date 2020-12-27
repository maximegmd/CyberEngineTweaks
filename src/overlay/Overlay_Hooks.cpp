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
        {
            spdlog::error("\tCould not hook mouse clip function!");
        }
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
        {
            spdlog::error("\tCould not hook Log function!");
        }
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
        {
            spdlog::error("\tCould not hook LogChannel function!");
        }
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
        {
            spdlog::error("\tCould not hook TDBID::ctor function!");
        }
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
        {
            spdlog::error("\tCould not hook TDBID::ctor[CString] function!");
        }
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
        {
            spdlog::error("\tCould not hook TDBID::ctor[Derive] function!");
        }
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
        {
            spdlog::error("\tCould not hook TDBID::ctor[Unknown] function!");
        }
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
            {
                spdlog::error("\tCould not hook TDBID::ToStringDEBUG function!");
            }
            else
                spdlog::info("\tTDBID::ToStringDEBUG function hook complete!");
        }
    }
}

long Overlay::PresentD3D12(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static std::once_flag s_init;
    std::call_once(s_init, [pSwapChain]()
    {
        Get().InitializeD3D12(pSwapChain);
    });

    Get().Render(pSwapChain);

    return Get().m_realPresentD3D12(pSwapChain, SyncInterval, Flags);
}

void Overlay::ExecuteCommandListsD3D12(ID3D12CommandQueue* apCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    auto& overlay = Get();
    if (overlay.m_pCommandQueue == nullptr)
    {
        auto desc = apCommandQueue->GetDesc();
        if(desc.Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
            overlay.m_pCommandQueue = apCommandQueue;
    }

    overlay.m_realExecuteCommandLists(apCommandQueue, NumCommandLists, ppCommandLists);
}

BOOL Overlay::ClipToCenter(RED4ext::REDreverse::CGameEngine::UnkC0* apThis)
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
    if (kiero::bind(140, reinterpret_cast<void**>(&m_realPresentD3D12), &PresentD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 Present Hook failed!");

    if (kiero::bind(54, reinterpret_cast<void**>(&m_realExecuteCommandLists), &ExecuteCommandListsD3D12) != kiero::Status::Success)
        spdlog::error("\tD3D12 ExecuteCommandLists Hook failed!");

    spdlog::info("\tD3D12 hook complete");
}

