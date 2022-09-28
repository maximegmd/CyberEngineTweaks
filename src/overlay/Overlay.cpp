#include <stdafx.h>

#include "Overlay.h"

#include "CET.h"
#include "widgets/HelperWidgets.h"

#include <Options.h>

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>

void Overlay::PostInitialize()
{
    if (!m_initialized)
    {
        if (m_options.IsFirstLaunch)
        {
            m_showFirstTimeModal = true;
            Toggle();
        }
        m_initialized = true;
    }
}

Console& Overlay::GetConsole()
{
    return m_console;
}

Bindings& Overlay::GetBindings()
{
    return m_bindings;
}

Settings& Overlay::GetSettings()
{
    return m_settings;
}

void Overlay::Toggle()
{
    if (!m_toggled)
        m_toggled = true;
}

bool Overlay::IsEnabled() const noexcept
{
    return m_initialized && m_enabled;
}

VKBind Overlay::GetBind() const noexcept
{
    return m_VKBIOverlay.Bind;
}

void Overlay::Update()
{
    if (!m_initialized)
        return;
    
    if (m_toggled)
    {
        if (m_enabled)
        {
            if (m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnDisable())
            {
                m_vm.OnOverlayClose();
                m_toggled = false;
                m_enabled = false;
            }
        }
        else
        {
            if (m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnEnable())
            {
                m_vm.OnOverlayOpen();
                m_toggled = false;
                m_enabled = true;
            }
        }

        if (!m_toggled)
        {
            auto& d3d12 = CET::Get().GetD3D12();
            d3d12.DelayedSetTrapInputInImGui(m_enabled);
            ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
        }
    }

    if (!m_enabled)
        return;

    if (m_options.IsFirstLaunch)
    {
        if (m_showFirstTimeModal)
        {
            assert(!m_VKBIOverlay.CodeBind);
            assert(!m_VKBIOverlay.SavedCodeBind);
            assert(!m_VKBIOverlay.IsBinding);

            ImGui::OpenPopup("CET First Time Setup");
            m_showFirstTimeModal = false;
            m_vm.BlockDraw(true);
        }

        if (ImGui::BeginPopupModal("CET First Time Setup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto shorterTextSz { ImGui::CalcTextSize("Combo can be composed from up to 4 keys.").x };
            const auto longerTextSz { ImGui::CalcTextSize("Please, bind some key combination for toggling overlay!").x };
            const auto diffTextSz { longerTextSz - shorterTextSz };

            ImGui::TextUnformatted("Please, bind some key combination for toggling overlay!");
            ImGui::SetCursorPosX(diffTextSz / 2);
            ImGui::TextUnformatted("Combo can be composed from up to 4 keys.");
            ImGui::Separator();

            // TODO - do not hardcode offset! this somewhat works temporarily...
            HelperWidgets::BindWidget(m_VKBIOverlay, false, diffTextSz * 0.75f);
            if (m_VKBIOverlay.CodeBind)
            {
                m_VKBIOverlay.Apply();
                m_bindings.Save();
                m_options.IsFirstLaunch = false;
                m_vm.BlockDraw(false);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        return;
    }

    auto& d3d12 = CET::Get().GetD3D12();
    const SIZE resolution = d3d12.GetResolution();

    ImGui::SetNextWindowPos(ImVec2(resolution.cx * 0.2f, resolution.cy * 0.2f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(resolution.cx * 0.6f, resolution.cy * 0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(420, 315), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Cyber Engine Tweaks"))
    {
        const ImVec2 cZeroVec = {0, 0};
        
        SetActiveWidget(HelperWidgets::ToolbarWidget());
        
        if (m_activeWidgetID == WidgetID::CONSOLE)
        {
            if (ImGui::BeginChild("Console", cZeroVec, true))
                m_console.Update();
            ImGui::EndChild();
        }
        if (m_activeWidgetID == WidgetID::BINDINGS)
        {
            if (ImGui::BeginChild("Bindings", cZeroVec, true))
                m_bindings.Update();
            ImGui::EndChild();
        }
        if (m_activeWidgetID == WidgetID::TWEAKDB)
        {
            if (ImGui::BeginChild("TweakDB Editor", cZeroVec, true))
                m_tweakDBEditor.Update();
            ImGui::EndChild();
        }
        if (m_activeWidgetID == WidgetID::SETTINGS)
        {
            if (ImGui::BeginChild("Settings", cZeroVec, true))
                m_settings.Update();
            ImGui::EndChild();
        }
    }
    ImGui::End();

    if (m_options.DrawImGuiDiagnosticWindow)
        ImGui::ShowMetricsWindow(&m_options.DrawImGuiDiagnosticWindow);
}

bool Overlay::IsInitialized() const noexcept
{
    return m_initialized;
}

LRESULT Overlay::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM)
{
    // TODO - is this useful now?
    return 0;
}

BOOL Overlay::ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
{
    HWND wnd = (HWND)apThis->hWnd;
    HWND foreground = GetForegroundWindow();

    auto& overlay = CET::Get().GetOverlay();

    if (wnd == foreground && apThis->unk164 && !apThis->unk154 && !overlay.IsEnabled())
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
    RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CWinapi_ClipToCenter);

    uint8_t* pLocation = func.GetAddr();

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &ClipToCenter, reinterpret_cast<void**>(&m_realClipToCenter)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            Log::Error("Could not hook mouse clip function!");
        else
            Log::Info("Hook mouse clip function!");
    }
}

Overlay::Overlay(D3D12& aD3D12, VKBindings& aBindings, Options& aOptions, LuaVM& aVm)
    : m_console(aVm)
    , m_bindings(aBindings, *this, aVm)
    , m_settings(aOptions, aVm)
    , m_tweakDBEditor(aVm)
    , m_d3d12(aD3D12)
    , m_options(aOptions)
    , m_vm(aVm)
{
    m_widgets[static_cast<size_t>(WidgetID::CONSOLE)] = &m_console;
    m_widgets[static_cast<size_t>(WidgetID::BINDINGS)] = &m_bindings;
    m_widgets[static_cast<size_t>(WidgetID::SETTINGS)] = &m_settings;
    m_widgets[static_cast<size_t>(WidgetID::TWEAKDB)] = &m_tweakDBEditor;

    Hook();

    m_options.IsFirstLaunch = !aBindings.Load(*this);

    m_connectInitialized = aD3D12.OnInitialized.Connect([this]() { PostInitialize(); });
    m_connectUpdate = aD3D12.OnUpdate.Connect([this]() { Update(); });

    
#ifdef _WIN32
    int SysLangID = GetSystemDefaultLangID();

    if (SysLangID == 1049)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF("RobotoCondensed.ttf", 14, nullptr,
                                     io.Fonts->GetGlyphRangesCyrillic());
    }
#endif
}

Overlay::~Overlay()
{
    m_d3d12.OnInitialized.Disconnect(m_connectInitialized);
    m_d3d12.OnUpdate.Disconnect(m_connectUpdate);
}

void Overlay::SetActiveWidget(WidgetID aNewActive)
{
    if (aNewActive < WidgetID::COUNT)
        m_nextActiveWidgetID = aNewActive;

    if (m_activeWidgetID != m_nextActiveWidgetID)
    {
        assert(m_activeWidgetID < WidgetID::COUNT);
        if (m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnDisable())
        {
            assert(m_nextActiveWidgetID < WidgetID::COUNT);
            if (m_widgets[static_cast<size_t>(m_nextActiveWidgetID)]->OnEnable())
                m_activeWidgetID = m_nextActiveWidgetID;
        }
    }
}
