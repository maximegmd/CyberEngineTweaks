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
            // properly auto-bind Toolbar if it is present here (could be this is first start so it may not be in here yet)
            auto vkBind = (it.key() == Toolbar::VKBToolbar.ID) ? (Toolbar::VKBToolbar) : (VKBind{ it.key() });
            auto ret = Bind(it.value(), vkBind);
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
    return _byteswap_ulong(*reinterpret_cast<const UINT*>(aVKCodeBindDecoded.data()));
}

bool VKBindings::StartRecordingBind(const VKBind& aBind)
{
    if (IsBindRecording)
        return false;

    Recording.fill(0);
    RecordingLength = 0;
    RecordingBind = aBind;
    RecordingResult = 0;
    IsBindRecording = true;

    return true;
}

bool VKBindings::StopRecordingBind()
{
    if (!IsBindRecording)
        return false;
    
    IsBindRecording = false;
    RecordingLength = 0;
    Recording.fill(0);

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

    switch (auMsg)
    {
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
LRESULT VKBindings::RecordKeyDown(UINT aVKCode)
{
    for (size_t i = 0; i < RecordingLength; ++i)
        if (Recording[i] == aVKCode)
            return 0; // ignore repeats

    if (RecordingLength >= Recording.size())
    {
        for (size_t i = 1; i < RecordingLength; ++i)
            Recording[i-1] = Recording[i];

        --RecordingLength;
    }

    Recording[RecordingLength++] = aVKCode;

    VerifyRecording(); // we doesnt care about result of this here, we just want it corrected :P

    return 0;
}

LRESULT VKBindings::RecordKeyUp(UINT aVKCode)
{
    for (size_t i = 0; i < RecordingLength; ++i)
    {
        if (Recording[i] == aVKCode)
        {
            RecordingResult = EncodeVKCodeBind(Recording);
            RecordingLength = i;
            for (; i < Recording.size(); ++i)
                Recording[i] = 0;
            while (!VerifyRecording()) {} // fix recording for future use, so user can use HKs like Ctrl+C, Ctrl+V without the need of pressing again Ctrl for example
            if (IsBindRecording)
            {
                if (!Bind(RecordingResult, RecordingBind))
                    RecordingResult = 0;
                IsBindRecording = false;
                return 0;
            }
            auto bind = Binds.find(RecordingResult);
            if (bind != Binds.end())
            {
                if (Toolbar::Get().IsEnabled() && (bind->second.ID != Toolbar::VKBToolbar.ID))
                    return 0; // we dont want to handle bindings if toolbar is open and we are not in binding state!

                if (bind->second.Handler) // prevention for freshly loaded bind from file without rebinding
                    bind->second.Handler();
            }
            return 0;
        }
    }
    // if we got here, aVKCode is not recorded key or there is no bind, so we ignore this event
    return 0;
}

bool VKBindings::VerifyRecording()
{
    if (IsBindRecording)
        return true; // always valid for bind recordings

    if (RecordingLength == 0)
        return true; // always valid when empty

    auto possibleBind = Binds.lower_bound(EncodeVKCodeBind(Recording));
    if (possibleBind == Binds.end())
    {
        Recording[--RecordingLength] = 0;
        return false; // seems like there is no possible HK here
    }

    VKCodeBindDecoded pbCodeDec = DecodeVKCodeBind(possibleBind->first);
    for (size_t i = 0; i < RecordingLength; ++i)
    {
        if (pbCodeDec[i] != Recording[i])
        {
            Recording[--RecordingLength] = 0;
            return false; // seems like there is no possible HK here
        }
    }

    return true; // valid recording
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
    {
        auto& kb = raw->data.keyboard;
        switch (kb.Message)
        {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                return RecordKeyDown(kb.VKey);
            case WM_KEYUP:
            case WM_SYSKEYUP:
                return RecordKeyUp(kb.VKey);
        }
    }
    else if (raw->header.dwType == RIM_TYPEMOUSE) 
    {
        if (IsBindRecording && (RecordingBind.ID == Toolbar::VKBToolbar.ID))
            return 0; // ignore mouse keys for toolbar key binding!

        auto& m = raw->data.mouse;
        switch (raw->data.mouse.usButtonFlags)
        {
            case RI_MOUSE_LEFT_BUTTON_DOWN:
                return RecordKeyDown(VK_LBUTTON);
            case RI_MOUSE_LEFT_BUTTON_UP:
                return RecordKeyUp(VK_LBUTTON);
            case RI_MOUSE_RIGHT_BUTTON_DOWN:
                return RecordKeyDown(VK_RBUTTON);
            case RI_MOUSE_RIGHT_BUTTON_UP:
                return RecordKeyUp(VK_RBUTTON);
            case RI_MOUSE_MIDDLE_BUTTON_DOWN:
                return RecordKeyDown(VK_MBUTTON);
            case RI_MOUSE_MIDDLE_BUTTON_UP:
                return RecordKeyUp(VK_MBUTTON);
            case RI_MOUSE_BUTTON_4_DOWN:
                return RecordKeyDown(VK_XBUTTON1);
            case RI_MOUSE_BUTTON_4_UP:
                return RecordKeyUp(VK_XBUTTON1);
            case RI_MOUSE_BUTTON_5_DOWN:
                return RecordKeyDown(VK_XBUTTON2);
            case RI_MOUSE_BUTTON_5_UP:
                return RecordKeyUp(VK_XBUTTON2);
        }
    }

    return 0; 
}
