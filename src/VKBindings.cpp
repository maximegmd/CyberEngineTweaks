#include <stdafx.h>

#include <toolbar/Toolbar.h>

void VKBindings::Initialize()
{
    Load(); // just load saved bindings in here (if any are present)
    Initialized = true;
}

void VKBindings::InitializeMods(std::vector<VKBindInfo>& aVKBindInfos)
{
    // first, check existing bindings and try to bind in case there is no binding
    for (auto& vkBindInfo : aVKBindInfos)
    {
        auto bind = GetBindCodeForID(vkBindInfo.Bind.ID);
        if (bind != 0)
        {
            auto ret = Bind(bind, vkBindInfo.Bind); // rebind
            assert(ret); // we never really want ret == false here!
            vkBindInfo.SavedCodeBind = bind;
            vkBindInfo.CodeBind = bind;
            continue;
        }
        // VKBind not bound yet - try to bind to default key (if any was chosen)
        if (vkBindInfo.CodeBind != 0)
        {
            auto ret = Bind(vkBindInfo.CodeBind, vkBindInfo.Bind);
            if (ret)
            {
                // we successfully bound!
                vkBindInfo.SavedCodeBind = vkBindInfo.CodeBind;
                continue;
            }
        }
        // Cannot be bound now, key is most probably bound to something
        vkBindInfo.SavedCodeBind = 0;
        vkBindInfo.CodeBind = 0;
    }

    // now, find all dead bindings
    std::vector<std::pair<std::string, UINT>> deadIDToBinds;
    for (auto& idToBind : IDToBinds)
    {
        // always ignore Toolbar bind here!
        if (idToBind.first == Toolbar::VKBToolbar.ID)
            continue;

        // TODO - try to avoid O(n^2) situation here
        bool found = false;
        for (auto& vkBindInfo : aVKBindInfos)
        {
            found = (idToBind.first == vkBindInfo.Bind.ID);
            if (found)
                break;
        }

        if (!found)
            deadIDToBinds.emplace_back(idToBind);
    }

    // and remove them
    for (auto& idToBind : deadIDToBinds)
    {
        IDToBinds.erase(idToBind.first);
        Binds.erase(idToBind.second);
    }

    // finally, save our filtered bindings back to not lose them
    Save();
}

void VKBindings::Shutdown()
{
    Save(); // just in case, save bindings on exit
}

bool VKBindings::IsInitialized()
{
    return Initialized;
}

void VKBindings::Load()
{
    std::ifstream ifs{ Paths::VKBindingsPath };
    if(ifs)
    {
        auto config = nlohmann::json::parse(ifs);
        for (auto& it : config.items())
        {
            auto ret = Bind(it.value(), { it.key() });
            assert(ret); // we want this to never fail!
        }
    }
    ifs.close();
}

void VKBindings::Save()
{
    nlohmann::json config;

    for (auto& idToBind : IDToBinds)
        config[idToBind.first.c_str()] = idToBind.second;

    std::ofstream ofs{ Paths::VKBindingsPath };
    ofs << config.dump(4) << std::endl;
}

bool VKBindings::Bind(UINT aVKCodeBind, const VKBind& aBind)
{
    // bind check 1
    {
        auto bind = Binds.find(aVKCodeBind);
        if (bind != Binds.end())
        {
            if (bind->second.ID == aBind.ID)
            {
                bind->second.Handler = aBind.Handler; // rebind
                return true;
            }
            return false; // combo already bound and it is not the same VKBind!
        }
    }

    // bind check 2 (includes unbind in case it is probably desired)
    {
        auto idToBind = IDToBinds.find(aBind.ID);
        if (idToBind != IDToBinds.end())
        {
            auto bind = Binds.find(idToBind->second);
            assert (bind != Binds.end()); // if these are not true for some reason, we have Fd up binding maps!
            assert (bind->second.ID == aBind.ID); // if these are not true for some reason, we have Fd up binding maps!
            if (bind->second.Handler == nullptr)
            {
                bind->second.Handler = aBind.Handler; // rebind
                return true;
            }
            // this is not a rebind, this is new combo assignment to handler
            Binds.erase(bind);
            IDToBinds.erase(idToBind);
            // dont return or anything in this case, continue!
        }
    }

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
    
UINT VKBindings::GetBindCodeForID(const std::string& aID)
{
    assert(!aID.empty()); // we never really want aID to be empty here... but leave some fallback for release!
    if (aID.empty())
        return 0;

    auto idToBind = IDToBinds.find(aID);
    if (idToBind == IDToBinds.end())
        return 0;

    return idToBind->second;
}

std::string VKBindings::GetIDForBindCode(UINT aVKCodeBind)
{
    assert(aVKCodeBind != 0); // we never really want aVKCodeBind == 0 here... but leave some fallback for release!
    if (aVKCodeBind == 0)
        return { };

    auto bind = Binds.find(aVKCodeBind);
    if (bind == Binds.end())
        return { };

    return bind->second.ID;
}

VKCodeBindDecoded VKBindings::DecodeVKCodeBind(UINT aVKCodeBind)
{
    if (aVKCodeBind == 0)
        return {};

    VKCodeBindDecoded res{ };
    *reinterpret_cast<UINT*>(res.data()) = _byteswap_ulong(aVKCodeBind);
    return res;
}

UINT VKBindings::EncodeVKCodeBind(const VKCodeBindDecoded& aVKCodeBindDecoded)
{
    assert(aVKCodeBindDecoded[0] != 0); // we never want 0 here!!!
    return _byteswap_ulong(*reinterpret_cast<const UINT*>(aVKCodeBindDecoded.data()));
}

bool VKBindings::StartRecordingBind(const VKBind& aBind)
{
    if (IsBindRecording)
        return false;

    Recording.fill(0);
    RecordingLength = 0;
    RecordingBind = aBind;
    IsBindRecording = true;

    return true;
}

bool VKBindings::StopRecordingBind()
{
    if (!IsBindRecording)
        return false;

    IsBindRecording = false;
    Recording.fill(0);
    RecordingLength = 0;

    return true;
}

bool VKBindings::IsRecordingBind()
{
    return IsBindRecording;
}

UINT VKBindings::GetLastRecordingResult()
{
    return RecordingResult;
}

LRESULT VKBindings::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM alParam)
{
    if (!Initialized)
        return 0; // we have not yet fully initialized!

    // change mouse messages to our liking, we will not pass this message to game window anyway
    // also functions as "early escape" for these events
    if ((auMsg == WM_MBUTTONDOWN) || (auMsg == WM_MBUTTONUP))
    {
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
    }
    
    switch (auMsg)
    {
        case WM_MBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            return RecordKeyDown(awParam);
        case WM_MBUTTONUP:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            return RecordKeyUp(awParam);
        case WM_CHAR:
            return IsLastRecordingKey(MapVirtualKey(awParam, MAPVK_VSC_TO_VK));
        case WM_INPUT:
            return HandleRAWInput(reinterpret_cast<HRAWINPUT>(alParam));
    }
    return 0;
}

bool VKBindings::IsLastRecordingKey(UINT aVKCode)
{
    if (RecordingLength == 0)
        return false;

    return (Recording[RecordingLength-1] == aVKCode);
}
bool VKBindings::RecordKeyDown(UINT aVKCode)
{
    for (size_t i = 0; i < RecordingLength; ++i)
        if (Recording[i] == aVKCode)
            return false; // ignore repeats

    if (RecordingLength >= Recording.size())
    {
        for (size_t i = 1; i < RecordingLength; ++i)
            Recording[i-1] = Recording[i];
        --RecordingLength;
    }

    Recording[RecordingLength] = aVKCode;
    ++RecordingLength;
    return IsBindRecording;
}

bool VKBindings::RecordKeyUp(UINT aVKCode)
{
    for (size_t i = 0; i < RecordingLength; ++i)
    {
        if (Recording[i] == aVKCode)
        {
            RecordingResult = CreateVKCodeBindFromRecording();
            Recording.fill(0);
            RecordingLength = 0;
            if (IsBindRecording)
            {
                if (!Bind(RecordingResult, RecordingBind))
                    RecordingResult = 0;
                IsBindRecording = false;
                return true;
            }
            auto bind = Binds.find(RecordingResult);
            if (bind != Binds.end())
            {
                if (Toolbar::Get().IsEnabled() && (bind->second.ID != Toolbar::VKBToolbar.ID))
                    return false; // we dont want to handle bindings if toolbar is open!

                if (bind->second.Handler) // prevention for freshly loaded bind from file without rebinding
                    bind->second.Handler();

                return true;
            }
            return false;
        }
    }
    // if we got here, aVKCode is not recorded key or there is no bind, so we ignore this event
    return false;
}

UINT VKBindings::CreateVKCodeBindFromRecording()
{
    assert(RecordingLength != 0); // we never really want RecordingLength == 0 here... but leave some fallback for release!
    if (RecordingLength == 0)
        return 0;

    return _byteswap_ulong(*reinterpret_cast<UINT*>(Recording.data()));
}

LRESULT VKBindings::HandleRAWInput(HRAWINPUT ahRAWInput)
{
    UINT dwSize;
    GetRawInputData(ahRAWInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

    auto lpb = std::make_unique<BYTE[]>(dwSize);
    if (GetRawInputData(ahRAWInput, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
         Logger::WarningToMain("VKBindings::HandleRAWInput() - GetRawInputData() does not return correct size !");

    auto* raw = reinterpret_cast<RAWINPUT*>(lpb.get());
    if (raw->header.dwType == RIM_TYPEKEYBOARD) 
        return IsBindRecording && IsLastRecordingKey(raw->data.keyboard.VKey);
    if (raw->header.dwType == RIM_TYPEMOUSE) 
    {
        switch (raw->data.mouse.usButtonFlags)
        {
            case RI_MOUSE_LEFT_BUTTON_DOWN:
            case RI_MOUSE_LEFT_BUTTON_UP:
                return IsBindRecording && IsLastRecordingKey(VK_LBUTTON);
            case RI_MOUSE_RIGHT_BUTTON_DOWN:
            case RI_MOUSE_RIGHT_BUTTON_UP:
                return IsBindRecording && IsLastRecordingKey(VK_RBUTTON);
            case RI_MOUSE_MIDDLE_BUTTON_DOWN:
            case RI_MOUSE_MIDDLE_BUTTON_UP:
                return IsBindRecording && IsLastRecordingKey(VK_MBUTTON);
            case RI_MOUSE_BUTTON_4_DOWN:
            case RI_MOUSE_BUTTON_4_UP:
                return IsBindRecording && IsLastRecordingKey(VK_XBUTTON1);
            case RI_MOUSE_BUTTON_5_DOWN:
            case RI_MOUSE_BUTTON_5_UP:
                return IsBindRecording && IsLastRecordingKey(VK_XBUTTON2);
        }
    }

    return 0; 
}
