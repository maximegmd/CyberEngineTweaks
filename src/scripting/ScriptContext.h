#pragma once

struct ScriptContext
{
	ScriptContext();
	~ScriptContext();

private:

	sol::environment m_env;
};