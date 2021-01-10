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

bool VKBindings::Bind(UINT aVKCodeBind, const VKBind& aBind)
{
    if (IsBound(aVKCodeBind))
        return false;

    UnBind(aBind.ID); // unbind by unique ID first in case there exists previous binding

    Binds[aVKCodeBind] = aBind;
    IDToBinds[aBind.ID] = aVKCodeBind;
    return true;
}

bool VKBindings::UnBind(UINT aVKCodeBind)
{
    auto bind = Binds.find(aVKCodeBind);
    if (bind == Binds.end())
        return false;

    IDToBinds.erase(bind->second.ID);
    Binds.erase(bind);
    return true;
}

bool VKBindings::UnBind(const std::string& aID)
{
    auto idToBind = IDToBinds.find(aID);
    if (idToBind == IDToBinds.end())
        return false;
    
    Binds.erase(idToBind->second);
    IDToBinds.erase(idToBind);
    return true;
}

bool VKBindings::IsBound(UINT aVKCodeBind)
{
    return Binds.count(aVKCodeBind);
}

bool VKBindings::IsBound(const std::string& aID)
{
    return IDToBinds.count(aID);
}

void VKBindings::StartRecordingBind(const VKBind& aBind)
{
    RecordingLength = 0;
    RecordingBind = aBind;
    IsRecording = true;
}

bool VKBindings::IsRecordingBind()
{
    return IsRecording;
}

UINT VKBindings::GetLastRecordingResult()
{
    return RecordingResult;
}

LRESULT VKBindings::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM)
{
    if (!IsInitialized)
        return 0; // we have not yet fully initialized!

    if (IsRecording)
    {
        switch (auMsg)
        {
            case WM_MBUTTONDOWN:
                switch (awParam)
                {
                    case MK_LBUTTON:
                        awParam = VK_LBUTTON;
                        break;
                    case MK_RBUTTON:
                        awParam = VK_RBUTTON;
                        break;
                    case MK_MBUTTON:
                        awParam = VK_MBUTTON;
                        break;
                    case MK_XBUTTON1:
                        awParam = VK_XBUTTON1;
                        break;
                    case MK_XBUTTON2:
                        awParam = VK_XBUTTON2;
                        break;
                    default:
                        return 0; // we are not interested in other params
                }
                // awParam == VK_XXXX now, pass through
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                RecordKeyDown(awParam);
                return 1;
            case WM_MBUTTONUP:
                switch (awParam)
                {
                    case MK_LBUTTON:
                        awParam = VK_LBUTTON;
                        break;
                    case MK_RBUTTON:
                        awParam = VK_RBUTTON;
                        break;
                    case MK_MBUTTON:
                        awParam = VK_MBUTTON;
                        break;
                    case MK_XBUTTON1:
                        awParam = VK_XBUTTON1;
                        break;
                    case MK_XBUTTON2:
                        awParam = VK_XBUTTON2;
                        break;
                    default:
                        return 0; // we are not interested in other params
                }
                // awParam == VK_XXXX now, pass through
            case WM_KEYUP:
            case WM_SYSKEYUP:
                RecordKeyUp(awParam);
                return 1;
            case WM_CHAR:
                if (IsLastRecordingKey(MapVirtualKey(awParam, MAPVK_VSC_TO_VK)))
                    return 1;
                break;
        }
        return 0;
    }

    if (Options::IsFirstLaunch)
        return 0; // do not handle anything until user sets his settings for the first time

    /*switch (auMsg)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (awParam == Options::ToolbarKey)
            {
                Toggle();
                return 1;
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (awParam == Options::ToolbarKey)
                return 1;
            break;
        case WM_CHAR:
            if (awParam == Options::ToolbarChar)
                return 1;
            break;
    }*/

    return 0;
}

bool VKBindings::IsLastRecordingKey(UINT aVKCode)
{
    if (RecordingLength == 0)
        return false;

    return (Recording[RecordingLength-1] == aVKCode);
}
void VKBindings::RecordKeyDown(UINT aVKCode)
{
    if (RecordingLength >= Recording.size())
    {
        Recording[0] = Recording[1];
        Recording[1] = Recording[2];
        Recording[2] = Recording[3];
        --RecordingLength;
    }
    RecordingUp[RecordingLength] = false;
    Recording[RecordingLength] = aVKCode;
    ++RecordingLength;
}

void VKBindings::RecordKeyUp(UINT aVKCode)
{
    size_t numUp = 0;
    for (size_t i = 0; i < RecordingLength; ++i)
    {
        if (Recording[i] == aVKCode)
            RecordingUp[i] = true;
        numUp = RecordingUp[i];
    }
    if (numUp == RecordingLength)
    {
        RecordingResult = CreateVKCodeBindFromRecording();
        if (!Bind(RecordingResult, RecordingBind))
            RecordingResult = 0;
        IsRecording = false;
    }
}

UINT VKBindings::CreateVKCodeBindFromRecording()
{
    assert(RecordingLength); // we never really want this to happen... but leave some fallback here!
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
