#pragma once

#include "ScriptContext.h"

struct ScriptStore
{
	ScriptStore();
	~ScriptStore();

	void LoadAll(sol::state_view aStateView);

	void TriggerOnUpdate() const;
	void TriggerOnInit() const;

	sol::object Get(const std::string& acName) const;

private:
	
	std::unordered_map<std::string, ScriptContext> m_contexts;
};