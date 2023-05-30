#pragma once

#include "Options.h"
#include "Paths.h"
#include "PersistentState.h"
#include "VKBindings.h"
#include "d3d12/D3D12.h"
#include "overlay/Overlay.h"
#include "scripting/LuaVM.h"
#include "common/CETTasks.h"
#include "Fonts.h"

#define _(...) CET::Get().GetI18n().Translate(__VA_ARGS__).c_str()

struct CET
{
    ~CET();

    static void Initialize();
    static void Shutdown();
    static CET& Get();

    const Paths& GetPaths() const noexcept;
    const Options& GetOptions() const noexcept;
    const PersistentState& GetPersistentState() const noexcept;
    D3D12& GetD3D12() noexcept;
    VKBindings& GetBindings() noexcept;
    Overlay& GetOverlay() noexcept;
    LuaVM& GetVM() noexcept;
    Fonts& GetFonts() noexcept;
    I18n& GetI18n() noexcept;

    static bool IsRunning() noexcept;

private:
    CET();

    Paths m_paths;
    Options m_options;
    PersistentState m_persistentState;
    VKBindings m_bindings;
    Window m_window;
    Fonts m_fonts;
    D3D12 m_d3d12;
    LuaVM m_vm;
    I18n m_i18n;
    Overlay m_overlay;
    CETTasks m_tasks;
};
