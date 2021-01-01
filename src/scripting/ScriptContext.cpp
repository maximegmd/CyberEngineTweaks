#include <stdafx.h>

#include "ScriptContext.h"


ScriptContext::ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath)
    : m_lua(aStateView)
    , m_env(aStateView, sol::create, aStateView.globals())
{
    auto path = acPath / "init.lua";
    auto result = m_lua.do_file("", m_env);
    if (result.valid())
        m_object = result;
}

ScriptContext::~ScriptContext()
{
}

