#pragma once

#include "Type.h"
#include "BasicTypes.h"

struct StrongReference : Type
{
    StrongReference(sol::state_view aView, StrongHandle aStrongHandle);
	~StrongReference();
	
protected:

	virtual RED4ext::REDreverse::Scripting::IScriptable* GetHandle();
	
private:
    StrongHandle m_strongHandle;
};
