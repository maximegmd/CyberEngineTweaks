#include "WeakReference.h"
#include <RED4ext/REDreverse/Scripting/IScriptable.hpp>

WeakReference::WeakReference(sol::state_view aView, WeakHandle aWeakHandle)
    : Type(aView, aWeakHandle.handle->GetClass())
    , m_weakHandle(aWeakHandle)
{
}

WeakReference::~WeakReference()
{
    // Someday maybe actually free memory
}

RED4ext::REDreverse::Scripting::IScriptable* WeakReference::GetHandle()
{
    return m_weakHandle.handle;
}
