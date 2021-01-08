#include <stdafx.h>

#include "Console.h"

#include <Image.h>
#include <Pattern.h>
#include <Options.h>

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>

static std::unique_ptr<Console> s_pConsole;

void Console::Initialize()
{
    if (!s_pConsole)
    {
        s_pConsole.reset(new (std::nothrow) Console);
        s_pConsole->Hook();
    }
}

void Console::Shutdown()
{
    s_pConsole = nullptr;
}

Console& Console::Get()
{
    return *s_pConsole;
}

void Console::Update()
{
    if (this == nullptr)
        return;

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
            Log(std::string("> ") + command);
            
            if (!LuaVM::Get().ExecuteLua(command))
                Log("Command failed to execute!");

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

    D3D12::Get().SetTrapInputInImGui(m_enabled);
    m_focusConsoleInput = m_enabled;

    auto& luaVM = LuaVM::Get();
    if (m_enabled)
        luaVM.OnConsoleOpen();
    else
        luaVM.OnConsoleClose();
    
    ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
}

bool Console::IsEnabled() const
{
    return m_enabled;
}

void Console::Log(const std::string& acpText)
{
    if (this == nullptr)
        return;

    std::lock_guard<std::recursive_mutex> _{ m_outputLock };
    std::istringstream lines(acpText);
    std::string line;

    while (std::getline(lines, line))
    {
        m_outputLines.emplace_back(line);
    }
    m_outputScroll = true;
}

LRESULT Console::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM)
{
    if (this == nullptr)
        return 0;

    auto& options = Options::Get();
    if (auMsg == WM_KEYDOWN && awParam == options.ConsoleKey)
    {
        Toggle();
        return 1;
    }

    switch (auMsg)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (awParam == options.ConsoleKey)
                return 1;
            break;
        case WM_CHAR:
            if (options.ConsoleChar && awParam == options.ConsoleChar)
                return 1;
            break;
    }

    if (IsEnabled())
    {
        if (auMsg == WM_KEYUP && awParam == VK_RETURN)
            m_focusConsoleInput = true;
    }

    return 0;
}

BOOL Console::ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
{
    HWND wnd = (HWND)apThis->hWnd;
    HWND foreground = GetForegroundWindow();

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

void Console::Hook()
{
    uint8_t* pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x8B,
        0x99, 0x68, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xF9, 0xFF });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &ClipToCenter, reinterpret_cast<void**>(&m_realClipToCenter)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook mouse clip function!");
        else
            spdlog::info("Hook mouse clip function!");
    }
}

Console::Console() = default;

Console::~Console() = default;
