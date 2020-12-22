#pragma once

#include "Type.h"

struct SingletonReference : Type
{
	SingletonReference(sol::state_view aView, RED4ext::REDreverse::CClass* apClass);
	~SingletonReference();
	
protected:

	virtual RED4ext::REDreverse::Scripting::IScriptable* GetHandle();
	
private:
    uint64_t m_hash;
};
