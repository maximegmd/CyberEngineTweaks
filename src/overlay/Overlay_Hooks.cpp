#include "Overlay.h"

#include <atlcomcli.h>
#include <d3d12.h>
#include <Image.h>
#include <imgui.h>
#include <MinHook.h>
#include <Pattern.h>
#include <kiero/kiero.h>
#include <spdlog/spdlog.h>

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
            spdlog::info("\LogChannel function hook complete!");
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

BOOL Overlay::ClipToCenter(CGameEngine::UnkC0* apThis)
{
    const HWND wnd = apThis->Wnd;
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
        spdlog::error("\tD3D12 PresentD3D12 Hook failed!");

    spdlog::info("\tD3D12 hook complete");
}

