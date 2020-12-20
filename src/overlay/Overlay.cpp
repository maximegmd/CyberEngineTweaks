#include "Overlay.h"

#include <atlcomcli.h>
#include <d3d12.h>
#include <Image.h>
#include <imgui.h>
#include <memory>
#include <Options.h>
#include <Pattern.h>
#include <kiero/kiero.h>
#include <spdlog/spdlog.h>

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "reverse/Engine.h"
#include "reverse/Scripting.h"

static std::shared_ptr<Overlay> s_pOverlay;

void Overlay::Initialize(Image* apImage)
{
    if (!s_pOverlay)
    {
        s_pOverlay.reset(new (std::nothrow) Overlay);
        s_pOverlay->EarlyHooks(apImage);
    }
}

void Overlay::Shutdown()
{
    s_pOverlay = nullptr;
}

Overlay& Overlay::Get()
{
    return *s_pOverlay;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Overlay::DrawImgui(IDXGISwapChain3* apSwapChain)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Cyber Engine Tweaks");

    if (Options::Get().GameImage.version == Image::MakeVersion(1, 4) ||
        Options::Get().GameImage.version == Image::MakeVersion(1, 5))
    {
        ImGui::PushItemWidth(600.f);

        static char command[512] = { 0 };

        {
            std::lock_guard<std::recursive_mutex> _{ m_outputLock };

            const auto result = ImGui::ListBoxHeader("", m_outputLines.size(), 15);
            for (auto& item : m_outputLines)
                ImGui::Selectable(item.c_str());

            if (result)
                ImGui::ListBoxFooter();
        }

        const auto execute = ImGui::InputText("", command, std::size(command), ImGuiInputTextFlags_EnterReturnsTrue);
        if (execute)
        {
            std::string returnMessage;

            if (!Scripting::Execute(command, returnMessage))
            {
                std::lock_guard<std::recursive_mutex> _{ m_outputLock };
                m_outputLines.push_back(std::string("Error ") + returnMessage);
            }
            else
            {
                std::lock_guard<std::recursive_mutex> _{ m_outputLock };
                m_outputLines.push_back(std::string(command) + std::string(" - success!"));
            }
        }
    }

    ImGui::End();

    ImGui::Render();
}

LRESULT APIENTRY Overlay::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
    case WM_KEYDOWN:
        if (wParam == Options::Get().ConsoleKey)
            s_pOverlay->Toggle();
		break;
    default:
        break;
    }

    if(s_pOverlay->IsEnabled() && uMsg != WM_PAINT && uMsg != WM_ACTIVATE && uMsg != WM_QUIT && uMsg != WM_CLOSE && uMsg != WM_DESTROY)
    {
        return true;
    }

    return CallWindowProc(s_pOverlay->m_wndProc, hwnd, uMsg, wParam, lParam);
}

using TScriptCall = void*(uint8_t*, uint8_t**, REDString*, void*);

TScriptCall** GetScriptCallArray()
{
    static uint8_t* pLocation = FindSignature({ 0x4C, 0x8D, 0x15, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x89, 0x42, 0x38, 0x49, 0x8B, 0xF8, 0x48, 0x8B, 0x02, 0x4C, 0x8D, 0x44, 0x24, 0x20, 0xC7 }) + 3;
    static uintptr_t finalLocation = (uintptr_t)pLocation + 4 + *reinterpret_cast<uint32_t*>(pLocation);

    return reinterpret_cast<TScriptCall**>(finalLocation);
}


void* Overlay::Log(uintptr_t apThis, uint8_t** apStack)
{
    REDString result("");
    apStack[6] = nullptr;
    apStack[7] = nullptr;
    auto stack = *(*apStack)++;
    GetScriptCallArray()[stack](apStack[8], apStack, &result, nullptr);
    ++(*apStack);

    std::lock_guard<std::recursive_mutex> _{ Get().m_outputLock };
    Get().m_outputLines.emplace_back(result.ToString());

    result.Destroy();

    return 0;
}

void Overlay::Toggle()
{
    struct Singleton
    {
        uint8_t pad0[0xC0];
        SomeStruct* pSomeStruct;
    };

    m_enabled = !m_enabled;

    while(true)
    {
        if (m_enabled && ShowCursor(TRUE) >= 0)
            break;
        if (!m_enabled && ShowCursor(FALSE) < 0)
            break;
    }

    ClipToCenter(CGameEngine::Get()->pSomeStruct);
}

bool Overlay::IsEnabled()
{
    return m_enabled;
}

Overlay::Overlay() = default;

Overlay::~Overlay() = default;

