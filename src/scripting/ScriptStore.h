#pragma once

#include "ScriptContext.h"

struct ScriptStore
{
	ScriptStore();
	~ScriptStore();

	void LoadAll();

private:
	
	std::unordered_map<std::filesystem::path, ScriptContext> m_contexts;
};