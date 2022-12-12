#include <stdafx.h>

#include "EngineTweaks.h"
#include "Options.h"

using namespace std::chrono_literals;

static std::unique_ptr<EngineTweaks> s_pInstance{nullptr};
static bool s_isRunning{true};

void EngineTweaks::Initialize()
{
    s_pInstance.reset(new EngineTweaks);
}

void EngineTweaks::Shutdown()
{
    s_pInstance.reset(nullptr);
}

EngineTweaks& EngineTweaks::Get()
{
    // we should always call this after initialization, never before!
    assert(s_pInstance);
    return *s_pInstance;
}

const Paths& EngineTweaks::GetPaths() const noexcept
{
    return m_paths;
}

const Options& EngineTweaks::GetOptions() const noexcept
{
    return m_options;
}

const PersistentState& EngineTweaks::GetPersistentState() const noexcept
{
    return m_persistentState;
}

D3D12& EngineTweaks::GetD3D12() noexcept
{
    return m_d3d12;
}

VKBindings& EngineTweaks::GetBindings() noexcept
{
    return m_bindings;
}

Overlay& EngineTweaks::GetOverlay() noexcept
{
    return m_overlay;
}

LuaVM& EngineTweaks::GetVM() noexcept
{
    return m_vm;
}

bool EngineTweaks::IsRunning() noexcept
{
    return s_isRunning;
}

EngineTweaks::EngineTweaks()
    : m_options(m_paths)
    , m_persistentState(m_paths, m_options)
    , m_bindings(m_paths, m_options)
    , m_window(&m_bindings, &m_d3d12)
    , m_d3d12(m_window, m_paths, m_options)
    , m_vm(m_paths, m_bindings, m_d3d12)
    , m_overlay(m_bindings, m_options, m_persistentState, m_vm)
{
    m_vm.Initialize();
}

EngineTweaks::~EngineTweaks()
{
    s_isRunning = false;
}
