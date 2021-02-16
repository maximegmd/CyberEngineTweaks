#pragma once

#include "Options.h"
#include "Paths.h"
#include "VKBindings.h"
#include "d3d12/D3D12.h"
#include "overlay/Overlay.h"
#include "scripting/LuaVM.h"
#include "common/CETTasks.h"

struct CET
{
    ~CET();

    static void Initialize();
    static void Shutdown();
    static CET& Get();

    const Paths& GetPaths() const noexcept;
    const Options& GetOptions() const noexcept;
    D3D12& GetD3D12() noexcept;
    VKBindings& GetBindings() noexcept;
    Overlay& GetOverlay() noexcept;
    LuaVM& GetVM() noexcept;

    static bool IsRunning() noexcept;

private:

    CET();
    
    Paths m_paths;
    Options m_options;
    VKBindings m_bindings;
    Window m_window;
    D3D12 m_d3d12;
    LuaVM m_vm;
    Overlay m_overlay;
    CETTasks m_tasks;
};
