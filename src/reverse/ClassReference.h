#pragma once

#include "Type.h"
#include "LuaRED.h"

struct ClassReference : Type
{
	ClassReference(sol::state_view aView, RED4ext::CClass* apClass, void* instance);
	~ClassReference();

	virtual RED4ext::IScriptable* GetHandle(); // This is likely not guaranteed to be an IScriptable should be "InstanceType"

private:
	void* m_instance;
};
