#include <stdafx.h>

void VKBindings::Initialize()
{
    
}

void VKBindings::Load()
{
    
}

void VKBindings::Save()
{
    
}

bool VKBindings::Bind(UINT aVKCodeBind, TVKBindCallback* aHandler)
{
    if (IsBound(aVKCodeBind))
        return false;

    return false;
}

bool VKBindings::IsBound(UINT aVKCodeBind)
{
    
}

bool VKBindings::StartRecordingBind(TVKBindCallback* aHandler)
{
    
}

void VKBindings::RecordKeyDown(UINT aVKCode)
{
    
}

void VKBindings::RecordKeyUp(UINT aVKCode)
{
    
}

bool VKBindings::IsRecordingBind()
{
    return IsRecording;
}

UINT VKBindings::CreateVKCodeBindFromRecording()
{
    if (RecordingLength == 0)
        return 0;

    UINT res = 0;
    auto* cur = Recording.data();
    auto shift = 24;
    do
    {
        res |= *(cur++) << shift;
        shift -= 8;
    }
    while (--RecordingLength);

    return res;
}
