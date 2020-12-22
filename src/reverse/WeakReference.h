#pragma once

#include "Type.h"
#include "BasicTypes.h"

struct WeakReference : Type
{
	WeakReference(sol::state_view aView, WeakHandle aWeakHandle);
	~WeakReference();
	
protected:

	virtual RED4ext::REDreverse::Scripting::IScriptable* GetHandle();
	
private:
    WeakHandle m_weakHandle;
};
