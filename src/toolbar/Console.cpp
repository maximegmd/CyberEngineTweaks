#include <stdafx.h>

#include "Console.h"

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>

void Console::Update()
{
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
        ImGui::SameLine();
        if (ImGui::Button("Reload All Mods"))
            LuaVM::Get().ReloadAllMods();

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
                        m_focusConsoleInput = true;
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
        if (m_focusConsoleInput)
        {
            ImGui::SetKeyboardFocusHere();
            m_focusConsoleInput = false;
        }
        const auto execute = ImGui::InputText("##InputCommand", command, std::size(command), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SetItemDefaultFocus();
        if (execute)
        {
            Logger::ToConsoleFmt("> {}", command);
            
            if (!LuaVM::Get().ExecuteLua(command))
                Logger::ToConsole("Command failed to execute!");

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
    m_focusConsoleInput = m_enabled;
}

bool Console::IsEnabled() const
{
    return m_enabled;
}

void Console::Log(std::string_view acpText)
{
    std::lock_guard<std::recursive_mutex> _{ m_outputLock };

    size_t first = 0;
    while (first < acpText.size())
    {
        const auto second = acpText.find_first_of('\n', first);

        if (second == std::string_view::npos)
        {
            m_outputLines.emplace_back(acpText.substr(first));
            break;
        }

        if (first != second)
            m_outputLines.emplace_back(acpText.substr(first, second-first));

        first = second + 1;
    }

    m_outputScroll = true;
}

void Console::GameLog(std::string_view acpText)
{
    if (!m_disabledGameLog)
        Logger::ToConsole(acpText);
}
