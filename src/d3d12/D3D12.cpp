#include <stdafx.h>

#include "D3D12.h"

#include <Image.h>
#include <Options.h>

#include <kiero/kiero.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>

#include <overlay/Overlay.h>

static std::unique_ptr<D3D12> s_pD3D12;

void D3D12::Initialize(Image* apImage)
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

LRESULT APIENTRY D3D12::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (Get().m_initialized)
    {
        if (Options::Get().Console)
        {
            auto res = Overlay::Get().OnWndProc(hWnd, uMsg, wParam, lParam);
            if (res)
                return 0; // Overlay wants this input ignored!
        }

        if (Get().m_passInputToImGui)
        {
            auto res = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
            if (res)
                return res;

            if (Get().m_catchInputInImGui) // TODO: look into io.WantCaptureMouse and io.WantCaptureKeyboard
            {
                // ignore mouse & keyboard events
                if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) ||
                    (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST))
                    return 0;

                // ignore specific messages
                switch (uMsg)
                {
                    case WM_INPUT:
                        return 0;
                }
            }
        }
    }
    
    return CallWindowProc(Get().m_wndProc, hWnd, uMsg, wParam, lParam);
}

D3D12::D3D12() = default;

D3D12::~D3D12() 
{
    if (m_hWnd != nullptr)
        SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_wndProc));

    if (m_initialized) 
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }
}
