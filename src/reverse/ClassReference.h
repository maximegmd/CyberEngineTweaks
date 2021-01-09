#pragma once

#include "Type.h"
#include "LuaRED.h"

struct ClassReference : ClassType
{
	ClassReference(sol::state_view aView, RED4ext::IRTTIType* apClass, RED4ext::ScriptInstance apInstance);

	virtual RED4ext::ScriptInstance GetHandle(); 

private:
	std::unique_ptr<uint8_t[]> m_pInstance;
};
