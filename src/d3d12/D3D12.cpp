#include <stdafx.h>

#include "D3D12.h"

#include <kiero/kiero.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>

static std::unique_ptr<D3D12> s_pD3D12;

void D3D12::Initialize()
{
    if (!s_pD3D12)
    {
        s_pD3D12.reset(new (std::nothrow) D3D12);
        std::thread t([]()
        {
            if (kiero::init() != kiero::Status::Success)
                spdlog::error("Kiero failed!");
            else
            {
                const char* d3d12type = (kiero::isDownLevelDevice()) ? ("D3D12on7") : ("D3D12");
                spdlog::info("Kiero initialized for {0}", d3d12type);
                Get().Hook();
            }
        });
        t.detach();
    }
}

void D3D12::Shutdown()
{
    s_pD3D12 = nullptr;

    kiero::shutdown();
}

D3D12& D3D12::Get()
{
    return *s_pD3D12;
}

LRESULT D3D12::OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam)
{
    auto& d3d12 = Get();
    if (d3d12.IsInitialized())
    {
        if (d3d12.m_passInputToImGui)
        {
            auto res = ImGui_ImplWin32_WndProcHandler(ahWnd, auMsg, awParam, alParam);
            if (res)
                return res;

            if (d3d12.m_catchInputInImGui) // TODO: look into io.WantCaptureMouse and io.WantCaptureKeyboard
            {
                // ignore mouse & keyboard events
                if ((auMsg >= WM_MOUSEFIRST && auMsg <= WM_MOUSELAST) ||
                    (auMsg >= WM_KEYFIRST && auMsg <= WM_KEYLAST))
                    return 1;

                // ignore specific messages
                switch (auMsg)
                {
                    case WM_INPUT:
                        return 1;
                }
            }
        }
    }
    
    return 0;
}

D3D12::D3D12() = default;

D3D12::~D3D12() 
{
    if (m_initialized) 
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
