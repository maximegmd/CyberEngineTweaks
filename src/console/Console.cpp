#include <stdafx.h>

#include "Console.h"

#include <Image.h>
#include <Options.h>
#include <scripting/Scripting.h>

static std::unique_ptr<Console> s_pOverlay;

void Console::Initialize(Image* apImage)
{
    if (!s_pOverlay)
    {
        s_pOverlay.reset(new (std::nothrow) Console);
        s_pOverlay->Hook(apImage);
    }
}

void Console::Shutdown()
{
    s_pOverlay = nullptr;
}

Console& Console::Get()
{
    return *s_pOverlay;
}

void Console::Render()
{
    if (m_logCount.load(std::memory_order_relaxed) < 2)
        return;

    Scripting::Get().GetStore().TriggerOnUpdate();

    if (!IsEnabled())
        return;

    SIZE resolution = D3D12::Get().GetResolution();

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(resolution.cx, resolution.cy * 0.3f), ImGuiCond_FirstUseEver);

    ImGui::Begin("Cyber Engine Tweaks");

    auto [major, minor] = Options::Get().GameImage.GetVersion();

    if (major == 1 && (minor >= 4 && minor <= 6))
    {
        ImGui::Checkbox("Clear Input", &m_inputClear);
        ImGui::SameLine();
        if (ImGui::Button("Clear Output"))
        {
            std::lock_guard<std::recursive_mutex> _{ m_outputLock };
            m_outputLines.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Scroll Output", &m_outputShouldScroll);
        ImGui::SameLine();
        ImGui::Checkbox("Disable Game Log", &m_disabledGameLog);

        static char command[200000] = { 0 };

        {
            std::lock_guard<std::recursive_mutex> _{ m_outputLock };

            ImVec2 listboxSize = ImGui::GetContentRegionAvail();
            listboxSize.y -= ImGui::GetFrameHeightWithSpacing();
            const auto result = ImGui::ListBoxHeader("##ConsoleHeader", listboxSize);
            ImGuiListClipper clipper;
            clipper.Begin(m_outputLines.size());
            while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) 
                {
                    auto& item = m_outputLines[i];
                    ImGui::PushID(i);
                    if (ImGui::Selectable(item.c_str()))
                    {
                        auto str = item;
                        if (item[0] == '>' && item[1] == ' ')
                            str = str.substr(2);

                        std::strncpy(command, str.c_str(), sizeof(command) - 1);
                        Get().m_focusConsoleInput = true;
                    }
                    ImGui::PopID();
                }

            if (m_outputScroll)
            {
                if (m_outputShouldScroll)
                    ImGui::SetScrollHereY();
                m_outputScroll = false;
            }
            if (result)
                ImGui::ListBoxFooter();
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (Get().m_focusConsoleInput)
        {
            ImGui::SetKeyboardFocusHere();
            Get().m_focusConsoleInput = false;
        }
        const auto execute = ImGui::InputText("##InputCommand", command, std::size(command), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SetItemDefaultFocus();
        if (execute)
        {
            Get().Log(std::string("> ") + command);

            Scripting::Get().ExecuteLua(command);

            if (m_inputClear)
                std::memset(command, 0, sizeof(command));
        }
    }
    else
        ImGui::Text("Unknown version, please update your game and the mod");

    ImGui::End();
}

void Console::Toggle()
{
    m_enabled = !m_enabled;

    auto& d3d12 = D3D12::Get();
    d3d12.PassInputToImGui(m_enabled);
    d3d12.CatchInputInImGui(m_enabled);

    while(true)
    {
        if (m_enabled && ShowCursor(TRUE) >= 0) 
        {
            m_focusConsoleInput = true;
            break;
        }
        if (!m_enabled && ShowCursor(FALSE) < 0)
            break;
    }

    ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
}

bool Console::IsEnabled() const
{
    return m_enabled;
}

void Console::Log(const std::string& acpText)
{
    std::lock_guard<std::recursive_mutex> _{ m_outputLock };
    std::istringstream lines(acpText);
    std::string line;

    while (std::getline(lines, line))
    {
        m_outputLines.emplace_back(line);
    }
    m_outputScroll = true;
}

LRESULT Console::OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_KEYDOWN && wParam == Options::Get().ConsoleKey)
    {
        Toggle();
        return 1;
    }

    switch (uMsg)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (wParam == Options::Get().ConsoleKey)
                return 1;
            break;
        case WM_CHAR:
            if (Options::Get().ConsoleChar && wParam == Options::Get().ConsoleChar)
                return 1;
            break;
    }

    if (IsEnabled())
    {
        if (uMsg == WM_KEYUP && wParam == VK_RETURN)
            m_focusConsoleInput = true;
    }

    return 0;
}

Console::Console() = default;

Console::~Console() = default;
