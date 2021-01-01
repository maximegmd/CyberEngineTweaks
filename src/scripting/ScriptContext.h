#pragma once

struct ScriptContext
{
	ScriptContext(sol::state_view aStateView, const std::filesystem::path& acPath);
	~ScriptContext();

private:

	sol::state_view m_lua;
	sol::environment m_env;
	sol::object m_object;
};