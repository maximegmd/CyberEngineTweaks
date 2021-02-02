#include <stdafx.h>

#include "CET.h"

#include <overlay/Overlay.h>

void VKBindInfo::Fill(UINT aVKCodeBind, const VKBind& aVKBind)
{
    Bind = aVKBind;
    CodeBind = aVKCodeBind;
    SavedCodeBind = aVKCodeBind;
    IsBinding = false;
}

UINT VKBindInfo::Apply()
{
    SavedCodeBind = CodeBind;
    return CodeBind;
}

VKBindings::VKBindings(Paths& aPaths)
    : m_paths(aPaths)
{
}

bool VKBindings::IsInitialized() const noexcept
{
    return m_initialized;
}

std::vector<VKBindInfo> VKBindings::InitializeMods(std::vector<VKBindInfo> aVKBindInfos)
{
    // first, check existing bindings and try to bind in case there is no binding
    for (auto& vkBindInfo : aVKBindInfos)
    {
        const auto bind = GetBindCodeForID(vkBindInfo.Bind.ID);
        if (bind != 0)
        {
            const auto ret = Bind(bind, vkBindInfo.Bind); // rebind
            assert(ret);                            // we never really want ret == false here!
            vkBindInfo.SavedCodeBind = bind;
            vkBindInfo.CodeBind = bind;
            continue;
        }
        // VKBind not bound yet - try to bind to default key (if any was chosen)
        if (vkBindInfo.CodeBind != 0)
        {
            const auto ret = Bind(vkBindInfo.CodeBind, vkBindInfo.Bind);
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
    for (auto& idToBind : m_idToBind)
    {
        // always ignore internal CET binds here!
        if (!idToBind.first.compare(0, 4, "cet."))
            continue;

        // TODO - try to avoid O(n^2) situation here
        auto found = false;
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
        m_idToBind.erase(idToBind.first);
        m_binds.erase(idToBind.second);
    }

    // finally, save our filtered bindings back to not lose them
    Save();

    // return corrected bindings
    return aVKBindInfos;
}

void VKBindings::Load(Overlay& aOverlay)
{
    std::ifstream ifs{ m_paths.VKBindings() };
    if (ifs)
    {
        auto config = nlohmann::json::parse(ifs);
        for (auto& it : config.items())
        {
            // properly auto-bind Overlay if it is present here (could be this is first start so it may not be in here yet)
            auto vkBind = (it.key() == aOverlay.GetBind().ID) ? aOverlay.GetBind() : VKBind{it.key()};
            const auto ret = Bind(it.value(), vkBind);
            assert(ret); // we want this to never fail!
        }
    }

    m_pOverlay = &aOverlay;
    m_initialized = true;
}

void VKBindings::Save()
{
    nlohmann::json config;

    for (auto& idToBind : m_idToBind)
        config[idToBind.first.c_str()] = idToBind.second;

    std::ofstream ofs{m_paths.VKBindings()};
    ofs << config.dump(4) << std::endl;
}

void VKBindings::Update()
{
    const std::lock_guard lock(m_queuedCallbacksLock);
    while (!m_queuedCallbacks.empty())
    {
        m_queuedCallbacks.front()();
        m_queuedCallbacks.pop();
    }
}

void VKBindings::Clear()
{
    m_binds.clear();
    m_recordingBind = {};
}

bool VKBindings::Bind(UINT aVKCodeBind, const VKBind& aBind)
{
    // bind check 1
    {
        auto bind = m_binds.find(aVKCodeBind);
        if (bind != m_binds.end())
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
        const auto idToBind = m_idToBind.find(aBind.ID);
        if (idToBind != m_idToBind.end())
        {
            auto bind = m_binds.find(idToBind->second);
            assert (bind != m_binds.end()); // if these are not true for some reason, we have Fd up binding maps!
            assert (bind->second.ID == aBind.ID); // if these are not true for some reason, we have Fd up binding maps!
            if (bind->second.Handler == nullptr)
            {
                bind->second.Handler = aBind.Handler; // rebind
                return true;
            }
            // this is not a rebind, this is new combo assignment to handler
            m_binds.erase(bind);
            m_idToBind.erase(idToBind);
            // dont return or anything in this case, continue!
        }
    }

    m_binds[aVKCodeBind] = aBind;
    m_idToBind[aBind.ID] = aVKCodeBind;
    return true;
}

bool VKBindings::UnBind(UINT aVKCodeBind)
{
    auto bind = m_binds.find(aVKCodeBind);
    if (bind == m_binds.end())
        return false;

    m_idToBind.erase(bind->second.ID);
    m_binds.erase(bind);
    return true;
}

bool VKBindings::UnBind(const std::string& aID)
{
    const auto idToBind = m_idToBind.find(aID);
    if (idToBind == m_idToBind.end())
        return false;
    
    m_binds.erase(idToBind->second);
    m_idToBind.erase(idToBind);
    return true;
}

bool VKBindings::IsBound(UINT aVKCodeBind) const
{
    return m_binds.count(aVKCodeBind);
}

bool VKBindings::IsBound(const std::string& aID) const
{
    return m_idToBind.count(aID);
}
    
UINT VKBindings::GetBindCodeForID(const std::string& aID)
{
    assert(!aID.empty()); // we never really want aID to be empty here... but leave some fallback for release!
    if (aID.empty())
        return 0;

    const auto idToBind = m_idToBind.find(aID);
    if (idToBind == m_idToBind.end())
        return 0;

    return idToBind->second;
}

std::string VKBindings::GetIDForBindCode(UINT aVKCodeBind)
{
    assert(aVKCodeBind != 0); // we never really want aVKCodeBind == 0 here... but leave some fallback for release!
    if (aVKCodeBind == 0)
        return { };

    const auto bind = m_binds.find(aVKCodeBind);
    if (bind == m_binds.end())
        return { };

    return bind->second.ID;
}

VKCodeBindDecoded VKBindings::DecodeVKCodeBind(UINT aVKCodeBind)
{
    if (aVKCodeBind == 0)
        return { };

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

bool VKBindings::IsRecordingBind() const
{
    return m_isBindRecording;
}

UINT VKBindings::GetLastRecordingResult() const
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

void VKBindings::ConnectUpdate(D3D12& aD3D12)
{
    m_connectUpdate = aD3D12.OnUpdate.Connect([this](){ this->Update(); });
}

void VKBindings::DisconnectUpdate(D3D12& aD3D12)
{
    aD3D12.OnUpdate.Disconnect(m_connectUpdate);
    m_connectUpdate = static_cast<size_t>(-1);
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

    while (m_recordingLength >= m_recording.size())
    {
        for (size_t i = 1; i < m_recording.size(); ++i)
            m_recording[i-1] = m_recording[i];

        --m_recordingLength;
    }

    m_recording[m_recordingLength++] = aVKCode;

    VerifyRecording(); // we don't care about result of this here, we just want it corrected :P

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

            const auto bind = m_binds.find(m_recordingResult);
            if (bind != m_binds.end())
            {
                if (m_pOverlay && m_pOverlay->IsEnabled() && (bind->second.ID != m_pOverlay->GetBind().ID))
                    return 0; // we dont want to handle bindings if toolbar is open and we are not in binding state!

                if (bind->second.Handler) // prevention for freshly loaded bind from file without rebinding
                {
                    if (!bind->second.ID.compare(0, 4, "cet."))
                        bind->second.Handler(); // we need to execute this immediately, otherwise cursor will not show on overlay toggle
                    else
                    {
                        const std::lock_guard lock(m_queuedCallbacksLock); 
                        m_queuedCallbacks.push(bind->second.Handler);
                    }
                }
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

    const auto possibleBind = m_binds.lower_bound(EncodeVKCodeBind(m_recording));
    if (possibleBind == m_binds.end())
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

    const auto lpb = std::make_unique<BYTE[]>(dwSize);
    if (GetRawInputData(ahRAWInput, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize )
         spdlog::warn("VKBindings::HandleRAWInput() - GetRawInputData() does not return correct size !");

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
        if (m_pOverlay && m_isBindRecording && (m_recordingBind.ID == m_pOverlay->GetBind().ID))
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
