#include <stdafx.h>

#include "RenderContext.h"
#include "Addresses.h"

RenderContext* RenderContext::GetInstance() noexcept
{
    static RED4ext::UniversalRelocPtr<RenderContext*> s_instance(CyberEngineTweaks::AddressHashes::CRenderGlobal_InstanceOffset);
    return *s_instance.GetAddr();
}
