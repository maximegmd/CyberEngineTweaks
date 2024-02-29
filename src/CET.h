#pragma once

#include "Options.h"
#include "Paths.h"
#include "PersistentState.h"
#include "VKBindings.h"
#include "d3d12/D3D12.h"
#include "overlay/Overlay.h"
#include "RED4ext/Api/Sdk.hpp"
#include "scripting/LuaVM.h"

struct CET
{
    ~CET();

    static void Initialize(const RED4ext::Sdk* aSdk);
    static void Shutdown();
    static CET& Get();

    const Paths& GetPaths() const noexcept;
    const Options& GetOptions() const noexcept;
    const PersistentState& GetPersistentState() const noexcept;
    D3D12& GetD3D12() noexcept;
    VKBindings& GetBindings() noexcept;
    Overlay& GetOverlay() noexcept;
    LuaVM& GetVM() noexcept;

    static bool IsRunning() noexcept;

private:
    CET(const RED4ext::Sdk* aSdk);

    Paths m_paths;
    Options m_options;
    PersistentState m_persistentState;
    VKBindings m_bindings;
    Window m_window;
    D3D12 m_d3d12;
    LuaVM m_vm;
    Overlay m_overlay;
};
