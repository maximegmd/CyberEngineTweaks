#include "stdafx.h"

#include "RenderContext.h"
#include "Addresses.h"

RenderContext* RenderContext::GetInstance() noexcept
{
    static RED4ext::RelocPtr<RenderContext*> s_instance(CyberEngineTweaks::Addresses::CRenderGlobal_InstanceOffset);
    return *s_instance.GetAddr();
}
