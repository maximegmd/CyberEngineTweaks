#include <stdafx.h>

#include "CET.h"
#include "Options.h"

using namespace std::chrono_literals;

static std::unique_ptr<CET> s_pInstance{ nullptr };
static bool s_isRunning{true};

void CET::Initialize()
{
    s_pInstance.reset(new CET);
}

void CET::Shutdown()
{
    s_pInstance.reset(nullptr);
}

CET& CET::Get()
{
    // we should always call this after initialization, never before!
    assert(s_pInstance);
    return *s_pInstance;
}

const Paths& CET::GetPaths() const noexcept
{
    return m_paths;
}

const Options& CET::GetOptions() const noexcept
{
    return m_options;
}

const PersistentState& CET::GetPersistentState() const noexcept
{
    return m_persistentState;
}

D3D12& CET::GetD3D12() noexcept
{
    return m_d3d12;
}

VKBindings& CET::GetBindings() noexcept
{
    return m_bindings;
}

Overlay& CET::GetOverlay() noexcept
{
    return m_overlay;
}

LuaVM& CET::GetVM() noexcept
{
    return m_vm;
}

bool CET::IsRunning() noexcept
{
    return s_isRunning;
}

CET::CET()
    : m_options(m_paths)
    , m_persistentState(m_paths, m_options)
    , m_bindings(m_paths, m_options)
    , m_window(&m_bindings, &m_d3d12)
    , m_d3d12(m_window, m_paths, m_options)
    , m_vm(m_paths, m_bindings, m_d3d12)
    , m_overlay(m_d3d12, m_bindings, m_options, m_persistentState, m_vm)
{
    m_vm.Initialize();
}

CET::~CET()
{
    s_isRunning = false;
}
