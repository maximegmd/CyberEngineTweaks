#include <stdafx.h>

#include <overlay/Overlay.h>

static std::unique_ptr<VKBindings> s_pVKBindings;

void VKBindings::Initialize()
{
    if (!s_pVKBindings)
    {
        s_pVKBindings.reset(new (std::nothrow) VKBindings);
        s_pVKBindings->Load();
        s_pVKBindings->m_initialized = true;
    }
}

void VKBindings::InitializeMods(std::vector<VKBindInfo>& aVKBindInfos)
{
    VKBindings& vkb = Get();

    // first, check existing bindings and try to bind in case there is no binding
    for (auto& vkBindInfo : aVKBindInfos)
    {
        auto bind = vkb.GetBindCodeForID(vkBindInfo.Bind.ID);
        if (bind != 0)
        {
            auto ret = vkb.Bind(bind, vkBindInfo.Bind); // rebind
            assert(ret); // we never really want ret == false here!
            vkBindInfo.SavedCodeBind = bind;
            vkBindInfo.CodeBind = bind;
            continue;
        }
        // VKBind not bound yet - try to bind to default key (if any was chosen)
        if (vkBindInfo.CodeBind != 0)
        {
            auto ret = vkb.Bind(vkBindInfo.CodeBind, vkBindInfo.Bind);
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
    for (auto& idToBind : vkb.IDToBinds)
    {
        // always ignore Overlay bind here!
        if (idToBind.first == Overlay::VKBOverlay.ID)
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
        vkb.IDToBinds.erase(idToBind.first);
        vkb.Binds.erase(idToBind.second);
    }

    // finally, save our filtered bindings back to not lose them
    vkb.Save();
}

void VKBindings::Shutdown()
{
    Get().Save(); // just in case, save bindings on exit
    s_pVKBindings.reset();
}

bool VKBindings::IsInitialized()
{
    return s_pVKBindings->m_initialized;
}

VKBindings& VKBindings::Get()
{
    assert(s_pVKBindings);
    assert(s_pVKBindings->m_initialized);

    return *s_pVKBindings;
}

void VKBindings::Load()
{
    std::ifstream ifs{ Paths::VKBindingsPath };
    if (ifs)
    {
        auto config = nlohmann::json::parse(ifs);
        for (auto& it : config.items())
        {
            // properly auto-bind Overlay if it is present here (could be this is first start so it may not be in here yet)
            auto vkBind = (it.key() == Overlay::VKBOverlay.ID) ? (Overlay::VKBOverlay) : (VKBind{ it.key() });
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
    if (m_isBindRecording)
        return false;

    m_recording.fill(0);
    m_recordingLength = 0;
    m_recordingBind = aBind;
    m_recordingResult = 0;
    m_isBindRecording = true;

    return true;
}

bool VKBindings::StopRecordingBind()
{
    if (!m_isBindRecording)
        return false;
    
    m_isBindRecording = false;
    m_recordingLength = 0;
    m_recording.fill(0);

    return true;
}

bool VKBindings::IsRecordingBind()
{
    return m_isBindRecording;
}

UINT VKBindings::GetLastRecordingResult()
{
    return m_recordingResult;
}

const char* VKBindings::GetSpecialKeyName(UINT aVKCode)
{
    switch (aVKCode)
    {
        case VK_LBUTTON:
            return "Mouse LB";
        case VK_RBUTTON:
            return "Mouse RB";
        case VK_MBUTTON:
            return "Mouse MB";
        case VK_XBUTTON1:
            return "Mouse X1";
        case VK_XBUTTON2:
            return "Mouse X2";
        case VK_BACK:
            return "Backspace";
        case VK_TAB:
            return "Tab";
        case VK_CLEAR:
            return "Clear";
        case VK_RETURN:
            return "Enter";
        case VK_SHIFT:
            return "Shift";
        case VK_CONTROL:
            return "Ctrl";
        case VK_MENU:
            return "Alt";
        case VK_PAUSE:
            return "Pause";
        case VK_CAPITAL:
            return "Caps Lock";
        case VK_ESCAPE:
            return "Esc";
        case VK_SPACE:
            return "Space";
        case VK_PRIOR:
            return "Page Up";
        case VK_NEXT:
            return "Page Down";
        case VK_END:
            return "End";
        case VK_HOME:
            return "Home";
        case VK_LEFT:
            return "Left Arrow";
        case VK_UP:
            return "Up Arrow";
        case VK_RIGHT:
            return "Right Arrow";
        case VK_DOWN:
            return "Down Arrow";
        case VK_SELECT:
            return "Select";
        case VK_PRINT:
            return "Print";
        case VK_EXECUTE:
            return "Execute";
        case VK_INSERT:
            return "Insert";
        case VK_DELETE:
            return "Delete";
        case VK_HELP:
            return "Help";
        case VK_NUMPAD0:
            return "Numpad 0";
        case VK_NUMPAD1:
            return "Numpad 1";
        case VK_NUMPAD2:
            return "Numpad 2";
        case VK_NUMPAD3:
            return "Numpad 3";
        case VK_NUMPAD4:
            return "Numpad 4";
        case VK_NUMPAD5:
            return "Numpad 5";
        case VK_NUMPAD6:
            return "Numpad 6";
        case VK_NUMPAD7:
            return "Numpad 7";
        case VK_NUMPAD8:
            return "Numpad 8";
        case VK_NUMPAD9:
            return "Numpad 9";
        case VK_F1:
            return "F1";
        case VK_F2:
            return "F2";
        case VK_F3:
            return "F3";
        case VK_F4:
            return "F4";
        case VK_F5:
            return "F5";
        case VK_F6:
            return "F6";
        case VK_F7:
            return "F7";
        case VK_F8:
            return "F8";
        case VK_F9:
            return "F9";
        case VK_F10:
            return "F10";
        case VK_F11:
            return "F11";
        case VK_F12:
            return "F12";
        case VK_NUMLOCK:
            return "Num Lock";
        case VK_SCROLL:
            return "Scroll Lock";
        default:
            return nullptr;
    }
}

LRESULT VKBindings::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM alParam)
{
    if (!m_initialized)
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
    if (m_recordingLength == 0)
        return false;

    return (m_recording[m_recordingLength-1] == aVKCode);
}
LRESULT VKBindings::RecordKeyDown(UINT aVKCode)
{
    for (size_t i = 0; i < m_recordingLength; ++i)
        if (m_recording[i] == aVKCode)
            return 0; // ignore repeats

    if (m_recordingLength >= m_recording.size())
    {
        for (size_t i = 1; i < m_recordingLength; ++i)
            m_recording[i-1] = m_recording[i];

        --m_recordingLength;
    }

    m_recording[m_recordingLength++] = aVKCode;

    VerifyRecording(); // we doesnt care about result of this here, we just want it corrected :P

    return 0;
}

LRESULT VKBindings::RecordKeyUp(UINT aVKCode)
{
    for (size_t i = 0; i < m_recordingLength; ++i)
    {
        if (m_recording[i] == aVKCode)
        {
            m_recordingResult = EncodeVKCodeBind(m_recording);
            m_recordingLength = i;
            for (; i < m_recording.size(); ++i)
                m_recording[i] = 0;
            while (!VerifyRecording()) {} // fix recording for future use, so user can use HKs like Ctrl+C, Ctrl+V without the need of pressing again Ctrl for example
            if (m_isBindRecording)
            {
                if (!Bind(m_recordingResult, m_recordingBind))
                    m_recordingResult = 0;
                m_isBindRecording = false;
                return 0;
            }
            auto bind = Binds.find(m_recordingResult);
            if (bind != Binds.end())
            {
                if (Overlay::Get().IsEnabled() && (bind->second.ID != Overlay::VKBOverlay.ID))
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
    if (m_isBindRecording)
        return true; // always valid for bind recordings

    if (m_recordingLength == 0)
        return true; // always valid when empty

    auto possibleBind = Binds.lower_bound(EncodeVKCodeBind(m_recording));
    if (possibleBind == Binds.end())
    {
        m_recording[--m_recordingLength] = 0;
        return false; // seems like there is no possible HK here
    }

    VKCodeBindDecoded pbCodeDec = DecodeVKCodeBind(possibleBind->first);
    for (size_t i = 0; i < m_recordingLength; ++i)
    {
        if (pbCodeDec[i] != m_recording[i])
        {
            m_recording[--m_recordingLength] = 0;
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
        if (m_isBindRecording && (m_recordingBind.ID == Overlay::VKBOverlay.ID))
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
