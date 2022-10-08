#include <stdafx.h>

#include <CET.h>

std::function<void()> VKBind::DelayedCall(bool isDown) const
{
    if (!isDown) // hotkeys only on key up
    {
        const auto* fn = std::get_if<std::function<TVKBindHotkeyCallback>>(&Handler);
        if (fn)
            return *fn;
    }

    {
        const auto* fn = std::get_if<std::function<TVKBindInputCallback>>(&Handler);
        if (fn)
            return std::bind(*fn, isDown);
    }

    assert(isDown); // nullptr should ever return only for key down events, in case binding is a hotkey
    return nullptr;
}

void VKBind::Call(bool isDown) const
{
    auto fn { DelayedCall(isDown) };
    if (fn)
        fn();
}

bool VKBind::IsHotkey() const
{
    return std::holds_alternative<std::function<TVKBindHotkeyCallback>>(Handler);
}

bool VKBind::IsInput() const
{
    return std::holds_alternative<std::function<TVKBindInputCallback>>(Handler);
}

bool VKBind::HasSimpleDescription() const
{
    return std::holds_alternative<std::string>(Description);
}

bool VKBind::HasComplexDescription() const
{
    return std::holds_alternative<std::function<void()>>(Description);
}

bool VKBind::operator==(const std::string& id) const
{
    return ID == id;
}

VKBindings::VKBindings(Paths& aPaths, const Options& acOptions)
    : m_paths(aPaths)
    , m_cOptions(acOptions)
{
    Load();
}

bool VKBindings::IsInitialized() const noexcept
{
    return m_initialized;
}

void VKBindings::InitializeMods(const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& acVKBinds)
{
    m_binds.clear();

    const auto& overlayToggleModBind = Bindings::GetOverlayToggleModBind();

    // first, check existing bindings and try to bind in case there is no binding
    for (auto& [vkModName, vkBinds] : acVKBinds)
    {
        VKModBind vkModBind;
        vkModBind.ModName = vkModName;
        auto& modBinds = vkBinds.get();
        for (auto& modBind : modBinds)
        {
            // check for existing binding
            vkModBind.ID = modBind.ID;
            const auto bind = GetBindCodeForModBind(vkModBind, true);
            if (!Bind(bind, vkModBind))
            {
                // register new binding
                m_modIdToBinds[vkModBind.ModName][vkModBind.ID] = 0;
            }
        }
    }

    // now, find all dead bindings
    TiltedPhoques::Vector<std::string> deadMods;
    for (const auto& [modName, modIdToBinds] : m_modIdToBinds)
    {
        // always ignore internal bindings
        if (modName == overlayToggleModBind.ModName)
            continue;

        // check if mod is present
        const auto currentModBindingsIt = acVKBinds.find(modName);
        if (currentModBindingsIt == acVKBinds.cend())
        {
            // when we want to filter dead bindings, mark mod for removal (default)
            if (m_cOptions.RemoveDeadBindings)
            {
                deadMods.emplace_back(modName);
                continue;
            }
        }

        // find all dead bindings for current mod
        TiltedPhoques::Vector<std::string> deadIDs;
        for (auto& idToBind : modIdToBinds)
        {
            auto found = false;
            if (currentModBindingsIt != acVKBinds.cend())
            {
                for (auto& vkBind : currentModBindingsIt->second.get())
                {
                    found = (idToBind.first == vkBind.ID);
                    if (found)
                    {
                        // test if bind is hotkey and if not, check if bind is valid for input (which means it has
                        // simple input key, not combo)
                        found = (vkBind.IsHotkey() || ((idToBind.second & 0xFFFF000000000000) == idToBind.second));
                        break; // we just reset found flag accordingly and exit here, we found valid entry, no need to
                               // continue regardless of result
                    }
                }
            }

            if (!found)
                deadIDs.emplace_back(idToBind.first);
        }

        // filter dead bindings if option is enabled (default)
        // register them otherwise to prevent rebinds
        for (const auto& deadId : deadIDs)
        {
            if (m_cOptions.RemoveDeadBindings)
                m_modIdToBinds[modName].erase(deadId);
            else
                Bind(m_modIdToBinds[modName][deadId], {modName, deadId});
        }
    }

    // and lastly, remove all dead mods (if we have any to remove)
    for (const auto& deadMod : deadMods)
        m_modIdToBinds.erase(deadMod);

    // lastly, insert CET overlay bind info
    const auto overlayBindCode{GetBindCodeForModBind(overlayToggleModBind, true)};
    if (!Bind(overlayBindCode, overlayToggleModBind))
        m_modIdToBinds[overlayToggleModBind.ModName][overlayToggleModBind.ID] = 0;

    // finally, save our filtered bindings back to not lose them
    Save();
}

void VKBindings::Load()
{
    StopRecordingBind();

    // try to load config
    if (std::ifstream ifs{m_paths.VKBindings()})
    {
        auto config{nlohmann::json::parse(ifs)};
        for (auto& mod : config.items())
        {
            const auto& modName{mod.key()};
            const auto concatPoint = modName.find('.');
            if (concatPoint < modName.length())
            {
                // load old config type
                auto modBindName = modName.substr(0, concatPoint);
                auto modBindId = modName.substr(concatPoint + 1);
                m_modIdToBinds[modBindName][modBindId] = mod.value();
            }
            else
            {
                // load new config type
                auto& modBinds{mod.value()};
                for (auto& bind : modBinds.items())
                    m_modIdToBinds[modName][bind.key()] = bind.value();
            }
        }
    }

    Save();

    m_initialized = true;
}

void VKBindings::Save()
{
    StopRecordingBind();

    nlohmann::json config;

    for (const auto& [modName, modIdToBinds] : m_modIdToBinds)
    {
        auto& node = config[modName];
        for (const auto& [id, bind] : modIdToBinds)
        {
            node[id] = bind;
        }
    }

    std::ofstream ofs{m_paths.VKBindings()};
    ofs << config.dump(4) << std::endl;
}

void VKBindings::Update()
{
    m_queuedCallbacks.Drain();
}

static bool FirstKeyMatches(uint64_t aFirst, uint64_t aSecond)
{
    return (aFirst & 0xFFFF000000000000ull) == (aSecond & 0xFFFF000000000000ull);
}

bool VKBindings::Bind(uint64_t aVKCodeBind, const VKModBind& acVKModBind)
{
    if (aVKCodeBind == 0)
        return false;

    const auto bind = m_binds.lower_bound(aVKCodeBind);
    if (bind != m_binds.end())
    {
        // check if code binds are equal
        if (bind->first == aVKCodeBind)
        {
            // if these do not match, code bind is already bound to other mod bind!
            return bind->second == acVKModBind;
        }

        // in this case, we may have found a hotkey or simple input starting with same key
        if (FirstKeyMatches(bind->first, aVKCodeBind))
        {
            // first char matches! lets check that both binds are hotkey in this case
            const auto isHotkey = [vm = m_cpVm](const VKModBind& vkModBind) {
                if (!vm)
                {
                    // this should never happen!
                    assert(false);
                    return false;
                }

                const auto bind = vm->GetBind(vkModBind);
                return bind && bind->IsHotkey();
            };

            if (!isHotkey(bind->second) || !isHotkey(acVKModBind))
            {
                // one of these is not a hotkey! simple inputs cannot start with same key as some
                // hotkey and vice versa!
                return false;
            }
        }
    }

    UnBind(acVKModBind);
    m_binds[aVKCodeBind] = acVKModBind;
    m_modIdToBinds[acVKModBind.ModName][acVKModBind.ID] = aVKCodeBind;
    return true;
}

bool VKBindings::UnBind(uint64_t aVKCodeBind)
{
    if (m_binds.contains(aVKCodeBind))
    {
        const auto& modBind = m_binds.at(aVKCodeBind);
        m_modIdToBinds.at(modBind.ModName).at(modBind.ID) = 0;
        m_binds.erase(aVKCodeBind);
        return true;
    }
    return false;
}

bool VKBindings::UnBind(const VKModBind& acVKModBind)
{
    const auto modBinds = m_modIdToBinds.find(acVKModBind.ModName);
    if (modBinds == m_modIdToBinds.cend())
        return false;

    auto& idToBinds = modBinds.value();
    const auto idToBind = idToBinds.find(acVKModBind.ID);
    if (idToBind == idToBinds.cend())
        return false;

    return UnBind(idToBind->second);
}

bool VKBindings::IsBound(uint64_t aVKCodeBind) const
{
    const auto bind = m_binds.find(aVKCodeBind);
    return bind != m_binds.end();
}

bool VKBindings::IsBound(const VKModBind& acVKModBind) const
{
    if (const auto modIdToBindsIt = m_modIdToBinds.find(acVKModBind.ModName); modIdToBindsIt != m_modIdToBinds.cend())
    {
        const auto& modIdToBinds = modIdToBindsIt->second;
        if (const auto bindIt = modIdToBinds.find(acVKModBind.ID); bindIt != modIdToBinds.cend())
            return IsBound(bindIt->second);
    }
    return false;
}

bool VKBindings::IsFirstKeyUsed(uint64_t aVKCodeBind) const
{
    const auto bind = m_binds.lower_bound(aVKCodeBind & 0xFFFF000000000000ull);
    return (bind != m_binds.end()) ? FirstKeyMatches(aVKCodeBind, bind->first) : false;
}

std::string VKBindings::GetBindString(uint64_t aVKCodeBind)
{
    if (aVKCodeBind == 0)
        return "Unbound";

    std::string bindStr;
    const auto bindDec{DecodeVKCodeBind(aVKCodeBind)};
    for (const auto vkCode : bindDec)
    {
        if (vkCode == 0)
            break;

        const char* specialName{GetSpecialKeyName(vkCode)};
        if (specialName)
            bindStr += specialName;
        else
        {
            const char vkChar{static_cast<char>(MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR))};
            if (vkChar != 0)
                bindStr += vkChar;
            else
                bindStr += "Unknown";
        }
        bindStr += " + ";
    }
    return bindStr.erase(bindStr.rfind(" + "));
}

std::string VKBindings::GetBindString(const VKModBind& acVKModBind) const
{
    return GetBindString(GetBindCodeForModBind(acVKModBind));
}

uint64_t VKBindings::GetBindCodeForModBind(const VKModBind& acVKModBind, bool aIncludeDead) const
{
    assert(!acVKModBind.ModName.empty()); // we never really want acModName to be empty here... but leave some fallback for release!
    assert(!acVKModBind.ID.empty());   // we never really want acID to be empty here... but leave some fallback for release!
    if (acVKModBind.ModName.empty() || acVKModBind.ID.empty())
        return 0;

    const auto modBinds = m_modIdToBinds.find(acVKModBind.ModName);
    if (modBinds == m_modIdToBinds.cend())
        return 0;

    const auto& idToBinds = modBinds.value();
    const auto idToBind = idToBinds.find(acVKModBind.ID);
    if (idToBind == idToBinds.cend())
        return 0;

    return aIncludeDead || IsBound(idToBind->second) ? idToBind->second : 0;
}

const VKModBind* VKBindings::GetModBindForBindCode(uint64_t aVKCodeBind) const
{
    assert(aVKCodeBind != 0); // we never really want aVKCodeBind == 0 here... but leave some fallback for release!
    if (aVKCodeBind == 0)
        return nullptr;

    const auto bind = m_binds.find(aVKCodeBind);
    if (bind == m_binds.cend())
        return nullptr;

    return &bind->second;
}

const VKModBind* VKBindings::GetModBindStartingWithBindCode(uint64_t aVKCodeBind) const
{
    assert(aVKCodeBind != 0); // we never really want aVKCodeBind == 0 here... but leave some fallback for release!
    if (aVKCodeBind == 0)
        return nullptr;

    const auto bind = m_binds.lower_bound(aVKCodeBind & 0xFFFF000000000000ull);
    if (bind == m_binds.cend())
        return nullptr;

    if (!FirstKeyMatches(bind->first, aVKCodeBind))
        return nullptr;

    return &bind->second;
}

VKCodeBindDecoded VKBindings::DecodeVKCodeBind(uint64_t aVKCodeBind)
{
    if (aVKCodeBind == 0)
        return {};

    VKCodeBindDecoded res{};
    const auto vkCodeBind = _byteswap_uint64(aVKCodeBind);
    std::memcpy(res.data(), &vkCodeBind, sizeof(uint64_t));
    for (auto& key : res)
        key = _byteswap_ushort(key);
    return res;
}

uint64_t VKBindings::EncodeVKCodeBind(VKCodeBindDecoded aVKCodeBindDecoded)
{
    for (auto& key : aVKCodeBindDecoded)
        key = _byteswap_ushort(key);
    uint64_t vkCodeBindEncoded = 0;
    std::memcpy(&vkCodeBindEncoded, aVKCodeBindDecoded.data(), sizeof(uint64_t));
    return _byteswap_uint64(vkCodeBindEncoded);
}

bool VKBindings::StartRecordingBind(const VKModBind& acVKModBind)
{
    if (m_isBindRecording)
        return false;

    m_recording.fill(0);
    m_recordingLength = 0;
    m_recordingModBind = acVKModBind;
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

uint64_t VKBindings::GetLastRecordingResult() const
{
    return m_recordingResult;
}

const char* VKBindings::GetSpecialKeyName(USHORT aVKCode)
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
    case VKBC_MWHEELUP:
        return "Mouse Wheel Up";
    case VKBC_MWHEELDOWN:
        return "Mouse Wheel Down";
    case VKBC_MWHEELLEFT:
        return "Mouse Wheel Left";
    case VKBC_MWHEELRIGHT:
        return "Mouse Wheel Right";
    default:
        return nullptr;
    }
}

LRESULT VKBindings::OnWndProc(HWND, UINT auMsg, WPARAM, LPARAM alParam)
{
    if (!m_initialized)
        return 0; // we have not yet fully initialized!

    switch (auMsg)
    {
    case WM_INPUT:
        return HandleRAWInput(reinterpret_cast<HRAWINPUT>(alParam));
    case WM_KILLFOCUS:
        // reset key states on focus loss, as otherwise, we end up with broken recording state
        m_keyStates.reset();
        m_isBindRecording = false;
        m_recordingResult = 0;
        m_recordingLength = 0;
        m_recording.fill(0);
    }
    return 0;
}

void VKBindings::ConnectUpdate(D3D12& aD3D12)
{
    m_connectUpdate = aD3D12.OnUpdate.Connect([this] { this->Update(); });
}

void VKBindings::DisconnectUpdate(D3D12& aD3D12)
{
    aD3D12.OnUpdate.Disconnect(m_connectUpdate);
    m_connectUpdate = static_cast<size_t>(-1);
}

bool VKBindings::IsLastRecordingKey(USHORT aVKCode) const
{
    if (m_recordingLength == 0)
        return false;

    return (m_recording[m_recordingLength - 1] == aVKCode);
}

LRESULT VKBindings::RecordKeyDown(USHORT aVKCode)
{
    if (m_keyStates[aVKCode])
        return 0; // ignore repeats

    // mark key down
    m_keyStates[aVKCode] = true;

    if (m_recordingLength >= m_recording.size())
    {
        assert(m_recordingLength == m_recording.size());
        for (size_t i = 1; i < m_recordingLength; ++i)
            m_recording[i - 1] = m_recording[i];
        --m_recordingLength;
    }

    m_recording[m_recordingLength++] = aVKCode;

    if (CheckRecording() >= 0)
        ExecuteRecording(true);

    return 0;
}

LRESULT VKBindings::RecordKeyUp(USHORT aVKCode)
{
    // ignore up event when we did not register down event
    if (!m_keyStates[aVKCode])
        return 0;

    // mark key up
    m_keyStates[aVKCode] = false;

    // handle simple inputs first
    if (!m_recordingLength)
    {
        m_recording[m_recordingLength++] = aVKCode;

        if (CheckRecording() > 0)
            ExecuteRecording(false);

        return 0;
    }

    // handle hotkeys
    for (size_t i = 0; i < m_recordingLength; ++i)
    {
        if (m_recording[i] == aVKCode)
        {
            m_recordingResult = EncodeVKCodeBind(m_recording);

            const auto wasRecording = m_isBindRecording;

            if (CheckRecording() >= 0)
                ExecuteRecording(false);

            if (!wasRecording)
            {
                // TODO - bug where you can bind HK to Ctrl, Ctrl+C and Ctrl+V.
                //        in mentioned case below, you will execute all 3 bindings, while wanted are only the last ones!

                // fix recording for future use, so user can use HKs like Ctrl+C, Ctrl+V without the need of pressing
                // again Ctrl for example
                m_recordingLength = i;

                for (; i < m_recording.size(); ++i)
                    m_recording[i] = 0;

                while (CheckRecording() < 0) {}
            }

            return 0;
        }
    }

    // if we got here, aVKCode is not recorded key or there is no bind, so we ignore this event
    return 0;
}

int32_t VKBindings::CheckRecording()
{
    m_recordingResult = EncodeVKCodeBind(m_recording);

    if (m_isBindRecording || m_recordingLength == 0)
        return 0; // always valid for bind recordings or when empty

    const auto possibleBind = m_binds.lower_bound(m_recordingResult);
    if (possibleBind == m_binds.end())
    {
        m_recording[--m_recordingLength] = 0;
        m_recordingResult = EncodeVKCodeBind(m_recording);
        return -1; // seems like there is no possible HK here
    }

    VKCodeBindDecoded pbCodeDec = DecodeVKCodeBind(possibleBind->first);
    for (size_t i = 0; i < m_recordingLength; ++i)
    {
        if (pbCodeDec[i] != m_recording[i])
        {
            m_recording[--m_recordingLength] = 0;
            m_recordingResult = EncodeVKCodeBind(m_recording);
            return -1; // seems like there is no possible HK here
        }
    }

    // valid recording, return 1 when there was match found and 0 when there is possible match
    return (m_recordingLength == 4) || !pbCodeDec[m_recordingLength];
}

void VKBindings::ExecuteRecording(bool aLastKeyDown)
{
    // VM must be running by this point! (doesn't have to be initialized fully)
    assert(m_cpVm != nullptr);

    if (m_isBindRecording)
    {
        const auto bind = m_cpVm->GetBind(m_recordingModBind);
        if (!aLastKeyDown || (bind && bind->IsInput()))
        {
            // this should never happen!
            assert(false);
            StopRecordingBind();
            m_recordingResult = 0;
        }

        return;
    }

    if (m_recordingResult == 0)
        return;

    const auto& possibleBind = m_binds.find(m_recordingResult)->second;
    const auto bind = m_cpVm->GetBind(possibleBind);
    if (bind)
    {
        // we dont want to handle bindings when vm is not initialized or when overlay is open and we are not in binding state!
        // only exception allowed is any CET bind
        const auto& overlayToggleModBind = Bindings::GetOverlayToggleModBind();
        const auto cetBind = possibleBind.ModName == overlayToggleModBind.ModName;
        if (!cetBind && (!m_cpVm->IsInitialized() || CET::Get().GetOverlay().IsEnabled()))
        {
            if (bind->IsInput())
            {
                m_recordingResult = 0;
                m_recording.fill(0);
                m_recordingLength = 0;
            }
            return;
        }

        // check first if this is a hotkey, as those should only execute when last key was up
        if (bind->IsHotkey() && aLastKeyDown)
            return;

        // execute CET binds immediately, otherwise cursor will not show on overlay toggle
        if (cetBind)
            bind->Call(aLastKeyDown);
        else
            m_queuedCallbacks.Add(bind->DelayedCall(aLastKeyDown));

        // reset recording for simple input and when last key was up
        if (bind->IsInput())
        {
            m_recordingResult = 0;
            m_recording.fill(0);
            m_recordingLength = 0;
        }
    }
}

LRESULT VKBindings::HandleRAWInput(HRAWINPUT ahRAWInput)
{
    UINT dwSize{0};
    GetRawInputData(ahRAWInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

    const auto lpb = std::make_unique<BYTE[]>(dwSize);
    if (GetRawInputData(ahRAWInput, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
        Log::Warn("VKBindings::HandleRAWInput() - GetRawInputData() does not return correct size!");

    const auto* raw = reinterpret_cast<const RAWINPUT*>(lpb.get());
    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        const auto& kb = raw->data.keyboard;
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
        if (m_isBindRecording && m_recordingModBind == Bindings::GetOverlayToggleModBind())
            return 0; // ignore mouse keys for toolbar key binding!

        const auto& m = raw->data.mouse;
        switch (m.usButtonFlags)
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
        case RI_MOUSE_WHEEL:
        {
            const USHORT key{static_cast<USHORT>(RI_MOUSE_WHEEL | ((m.usButtonData & 0x8000) ? 0 : 1))};
            RecordKeyDown(key);
            return RecordKeyUp(key);
        }
        case RI_MOUSE_HWHEEL:
        {
            const USHORT key{static_cast<USHORT>(RI_MOUSE_HWHEEL | ((m.usButtonData & 0x8000) ? 0 : 1))};
            RecordKeyDown(key);
            return RecordKeyUp(key);
        }
        }
    }

    return 0;
}

void VKBindings::SetVM(const LuaVM* acpVm)
{
    // this should happen only once
    assert(m_cpVm == nullptr);
    // vm pointer shall be always valid when this is called!
    assert(acpVm != nullptr);

    m_cpVm = acpVm;
}
