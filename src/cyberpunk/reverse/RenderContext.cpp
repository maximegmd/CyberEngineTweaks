#include <stdafx.h>

#include "RenderContext.h"
#include "Addresses.h"

RenderContext* RenderContext::GetInstance() noexcept
{
    static RelocPtr<RenderContext*> s_instance(Game::Addresses::CRenderGlobal_InstanceOffset);
    return *s_instance.GetAddr();
}
