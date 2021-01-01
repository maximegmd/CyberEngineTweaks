#pragma once

#include "ScriptContext.h"

struct ScriptStore
{
	ScriptStore();
	~ScriptStore();

	void LoadAll(sol::state_view aStateView);

private:
	
	std::unordered_map<std::string, ScriptContext> m_contexts;
};