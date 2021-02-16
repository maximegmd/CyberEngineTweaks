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
    , m_bindings(m_paths)
    , m_window(&m_overlay, &m_bindings, &m_d3d12)
    , m_d3d12(m_window, m_paths, m_options)
    , m_vm(m_paths, m_bindings, m_d3d12, m_options)
    , m_overlay(m_d3d12, m_bindings, m_options, m_vm)
    , m_tasks(m_options)
{
    m_bindings.Bind(m_options.OverlayKeyBind, m_overlay.GetBind());
    m_bindings.ConnectUpdate(m_d3d12);

    m_vm.Initialize();
}

CET::~CET()
{
    s_isRunning = false;

    m_bindings.DisconnectUpdate(m_d3d12);
    m_bindings.Clear();
}
