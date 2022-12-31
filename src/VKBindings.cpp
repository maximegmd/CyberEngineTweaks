#include "stdafx.h"

#include "CET.h"
#include "Utils.h"

std::function<void()> VKBind::DelayedCall(const bool acIsDown) const
{
    if (!acIsDown) // hotkeys only on key up
    {
        if (const auto* fn = std::get_if<std::function<TVKBindHotkeyCallback>>(&Handler))
            return *fn;
    }

    if (const auto* fn = std::get_if<std::function<TVKBindInputCallback>>(&Handler))
        return [cb = *fn, acIsDown]
        {
            cb(acIsDown);
        };

    assert(acIsDown); // nullptr should ever return only for key down events, in case binding is a hotkey
    return nullptr;
}

void VKBind::Call(const bool acIsDown) const
{
    if (const auto fn{DelayedCall(acIsDown)})
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

bool VKBind::operator==(const std::string& acpId) const
{
    return ID == acpId;
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
        for (auto& modBind : vkBinds.get())
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
            if (m_cOptions.Developer.RemoveDeadBindings)
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
                    found = idToBind.first == vkBind.ID;
                    if (found)
                    {
                        // test if bind is hotkey and if not, check if bind is valid for input (which means it has
                        // simple input key, not combo)
                        found = vkBind.IsHotkey() || (idToBind.second & 0xFFFF000000000000) == idToBind.second;
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
            if (m_cOptions.Developer.RemoveDeadBindings)
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
    const auto path = GetAbsolutePath(m_paths.VKBindings(), "", false);
    if (std::ifstream ifs{path})
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
                for (auto& bind : mod.value().items())
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

    const auto path = GetAbsolutePath(m_paths.VKBindings(), "", true);
    std::ofstream ofs{path};
    ofs << config.dump(4) << std::endl;
}

void VKBindings::Update()
{
    m_queuedCallbacks.Drain();
}

static bool FirstKeyMatches(const uint64_t acFirst, const uint64_t acSecond)
{
    return (acFirst & 0xFFFF000000000000ull) == (acSecond & 0xFFFF000000000000ull);
}

bool VKBindings::Bind(const uint64_t acVKCodeBind, const VKModBind& acVKModBind)
{
    if (acVKCodeBind == 0)
        return false;

    const auto bind = m_binds.lower_bound(acVKCodeBind);
    if (bind != m_binds.end())
    {
        // check if code binds are equal
        if (bind->first == acVKCodeBind)
        {
            // if these do not match, code bind is already bound to other mod bind!
            return bind->second == acVKModBind;
        }

        // in this case, we may have found a hotkey or simple input starting with same key
        if (FirstKeyMatches(bind->first, acVKCodeBind))
        {
            // first char matches! lets check that both binds are hotkey in this case
            const auto isHotkey = [vm = m_cpVm](const VKModBind& vkModBind)
            {
                if (!vm)
                {
                    // this should never happen!
                    assert(false);
                    return false;
                }

                const auto vkBind = vm->GetBind(vkModBind);
                return vkBind && vkBind->IsHotkey();
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
    m_binds[acVKCodeBind] = acVKModBind;
    m_modIdToBinds[acVKModBind.ModName][acVKModBind.ID] = acVKCodeBind;
    return true;
}

bool VKBindings::UnBind(const uint64_t acVKCodeBind)
{
    if (m_binds.contains(acVKCodeBind))
    {
        const auto& modBind = m_binds.at(acVKCodeBind);
        m_modIdToBinds.at(modBind.ModName).at(modBind.ID) = 0;
        m_binds.erase(acVKCodeBind);
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

bool VKBindings::IsBound(const uint64_t acVKCodeBind) const
{
    const auto bind = m_binds.find(acVKCodeBind);
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

bool VKBindings::IsFirstKeyUsed(const uint64_t acVKCodeBind) const
{
    const auto bind = m_binds.lower_bound(acVKCodeBind & 0xFFFF000000000000ull);
    return bind != m_binds.end() ? FirstKeyMatches(acVKCodeBind, bind->first) : false;
}

std::string VKBindings::GetBindString(const uint64_t acVKCodeBind)
{
    if (acVKCodeBind == 0)
        return "Unbound";

    std::string bindStr;
    const auto bindDec{DecodeVKCodeBind(acVKCodeBind)};
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

uint64_t VKBindings::GetBindCodeForModBind(const VKModBind& acVKModBind, const bool acIncludeDead) const
{
    assert(!acVKModBind.ModName.empty()); // we never really want acModName to be empty here... but leave some fallback for release!
    assert(!acVKModBind.ID.empty());      // we never really want acID to be empty here... but leave some fallback for release!
    if (acVKModBind.ModName.empty() || acVKModBind.ID.empty())
        return 0;

    const auto modBinds = m_modIdToBinds.find(acVKModBind.ModName);
    if (modBinds == m_modIdToBinds.cend())
        return 0;

    const auto& idToBinds = modBinds.value();
    const auto idToBind = idToBinds.find(acVKModBind.ID);
    if (idToBind == idToBinds.cend())
        return 0;

    return acIncludeDead || IsBound(idToBind->second) ? idToBind->second : 0;
}

const VKModBind* VKBindings::GetModBindForBindCode(const uint64_t acVKCodeBind) const
{
    assert(acVKCodeBind != 0); // we never really want aVKCodeBind == 0 here... but leave some fallback for release!
    if (acVKCodeBind == 0)
        return nullptr;

    const auto bind = m_binds.find(acVKCodeBind);
    if (bind == m_binds.cend())
        return nullptr;

    return &bind->second;
}

const VKModBind* VKBindings::GetModBindStartingWithBindCode(const uint64_t acVKCodeBind) const
{
    assert(acVKCodeBind != 0); // we never really want aVKCodeBind == 0 here... but leave some fallback for release!
    if (acVKCodeBind == 0)
        return nullptr;

    const auto bind = m_binds.lower_bound(acVKCodeBind & 0xFFFF000000000000ull);
    if (bind == m_binds.cend())
        return nullptr;

    if (!FirstKeyMatches(bind->first, acVKCodeBind))
        return nullptr;

    return &bind->second;
}

VKCodeBindDecoded VKBindings::DecodeVKCodeBind(const uint64_t acVKCodeBind)
{
    if (acVKCodeBind == 0)
        return {};

    VKCodeBindDecoded res{};
    const auto vkCodeBind = _byteswap_uint64(acVKCodeBind);
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

    ClearRecording(true);

    m_recordingModBind = acVKModBind;
    m_isBindRecording = true;

    return true;
}

bool VKBindings::StopRecordingBind()
{
    if (!m_isBindRecording)
        return false;

    ClearRecording(false);

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

const char* VKBindings::GetSpecialKeyName(const USHORT acVKCode)
{
    switch (acVKCode)
    {
    case VK_LBUTTON: return "Mouse LB";
    case VK_RBUTTON: return "Mouse RB";
    case VK_MBUTTON: return "Mouse MB";
    case VK_XBUTTON1: return "Mouse X1";
    case VK_XBUTTON2: return "Mouse X2";
    case VK_BACK: return "Backspace";
    case VK_TAB: return "Tab";
    case VK_CLEAR: return "Clear";
    case VK_RETURN: return "Enter";
    case VK_SHIFT: return "Shift";
    case VK_CONTROL: return "Ctrl";
    case VK_MENU: return "Alt";
    case VK_PAUSE: return "Pause";
    case VK_CAPITAL: return "Caps Lock";
    case VK_ESCAPE: return "Esc";
    case VK_SPACE: return "Space";
    case VK_PRIOR: return "Page Up";
    case VK_NEXT: return "Page Down";
    case VK_END: return "End";
    case VK_HOME: return "Home";
    case VK_LEFT: return "Left Arrow";
    case VK_UP: return "Up Arrow";
    case VK_RIGHT: return "Right Arrow";
    case VK_DOWN: return "Down Arrow";
    case VK_SELECT: return "Select";
    case VK_PRINT: return "Print";
    case VK_EXECUTE: return "Execute";
    case VK_INSERT: return "Insert";
    case VK_DELETE: return "Delete";
    case VK_HELP: return "Help";
    case VK_NUMPAD0: return "Numpad 0";
    case VK_NUMPAD1: return "Numpad 1";
    case VK_NUMPAD2: return "Numpad 2";
    case VK_NUMPAD3: return "Numpad 3";
    case VK_NUMPAD4: return "Numpad 4";
    case VK_NUMPAD5: return "Numpad 5";
    case VK_NUMPAD6: return "Numpad 6";
    case VK_NUMPAD7: return "Numpad 7";
    case VK_NUMPAD8: return "Numpad 8";
    case VK_NUMPAD9: return "Numpad 9";
    case VK_F1: return "F1";
    case VK_F2: return "F2";
    case VK_F3: return "F3";
    case VK_F4: return "F4";
    case VK_F5: return "F5";
    case VK_F6: return "F6";
    case VK_F7: return "F7";
    case VK_F8: return "F8";
    case VK_F9: return "F9";
    case VK_F10: return "F10";
    case VK_F11: return "F11";
    case VK_F12: return "F12";
    case VK_NUMLOCK: return "Num Lock";
    case VK_SCROLL: return "Scroll Lock";
    case VKBC_MWHEELUP: return "Mouse Wheel Up";
    case VKBC_MWHEELDOWN: return "Mouse Wheel Down";
    case VKBC_MWHEELLEFT: return "Mouse Wheel Left";
    case VKBC_MWHEELRIGHT: return "Mouse Wheel Right";
    default: return nullptr;
    }
}

LRESULT VKBindings::OnWndProc(HWND, UINT auMsg, WPARAM, LPARAM alParam)
{
    if (!m_initialized)
        return 0; // we have not yet fully initialized!

    switch (auMsg)
    {
    case WM_INPUT: return HandleRAWInput(reinterpret_cast<HRAWINPUT>(alParam));
    case WM_KILLFOCUS:
        // reset key states on focus loss, as otherwise, we end up with broken recording state
        m_keyStates.reset();
        ClearRecording(false);
    }

    return 0;
}

LRESULT VKBindings::RecordKeyDown(const USHORT acVKCode)
{
    if (m_keyStates[acVKCode])
        return 0; // ignore repeats

    // mark key down
    m_keyStates[acVKCode] = true;

    ExecuteSingleInput(acVKCode, true);

    if (m_recordingLength != 0)
    {
        if (m_recordingLength >= m_recording.size())
        {
            ClearRecording(false);
            m_recordingResult = 0;

            return 0;
        }

        for (size_t i = 0; i < m_recordingLength; ++i)
        {
            assert(m_recording[i] != acVKCode);
            if (m_recording[i] == acVKCode)
                return 0;
        }
    }
    else if (m_keyStates.count() != 1)
        return 0;

    m_recording[m_recordingLength++] = acVKCode;
    m_recordingResult = EncodeVKCodeBind(m_recording);
    m_recordingWasKeyPressed = true;

    return 0;
}

LRESULT VKBindings::RecordKeyUp(const USHORT acVKCode)
{
    // ignore up event when we did not register down event
    if (!m_keyStates[acVKCode])
        return 0;

    // mark key up
    m_keyStates[acVKCode] = false;

    ExecuteSingleInput(acVKCode, false);

    for (size_t i = 0; i < m_recordingLength; ++i)
    {
        if (m_recording[i] != acVKCode)
            continue;

        if (m_recordingWasKeyPressed)
            ExecuteRecording();

        // fix recording for future use, so user can use HKs like Ctrl+C, Ctrl+V without the need of pressing
        // again Ctrl for example
        m_recordingLength = i;
        for (; i < m_recording.size(); ++i)
            m_recording[i] = 0;

        m_recordingWasKeyPressed = false;

        return 0;
    }

    return 0;
}

void VKBindings::ExecuteSingleInput(const USHORT acVKCode, const bool acKeyDown)
{
    // ignore single inputs when we are recording bind!
    if (m_isBindRecording)
        return;

    const auto cInput = EncodeVKCodeBind({acVKCode, 0, 0, 0});

    if (const auto bindIt = m_binds.find(cInput); bindIt != m_binds.cend())
    {
        if (bindIt->first != cInput)
            return;

        const auto& modBind = bindIt->second;
        if (const auto vkBind = m_cpVm->GetBind(modBind))
        {
            // we dont want to handle bindings when vm is not initialized or when overlay is open and we are not in
            // binding state! only exception allowed is any CET bind
            const auto& overlayToggleModBind = Bindings::GetOverlayToggleModBind();
            const auto cetModBind = modBind.ModName == overlayToggleModBind.ModName;
            if (!cetModBind && (!m_cpVm->IsInitialized() || CET::Get().GetOverlay().IsEnabled()))
                return;

            // this handler is not for hotkeys!
            if (vkBind->IsHotkey())
                return;

            // execute CET binds immediately, otherwise cursor will not show on overlay toggle
            if (cetModBind)
                vkBind->Call(acKeyDown);
            else
                m_queuedCallbacks.Add(vkBind->DelayedCall(acKeyDown));
        }
    }
}

void VKBindings::ExecuteRecording()
{
    if (m_isBindRecording)
    {
        m_isBindRecording = false;
        return;
    }

    if (const auto bindIt = m_binds.find(m_recordingResult); bindIt != m_binds.cend())
    {
        if (bindIt->first != m_recordingResult)
            return;

        const auto& modBind = bindIt->second;
        if (const auto vkBind = m_cpVm->GetBind(modBind))
        {
            // we dont want to handle bindings when vm is not initialized or when overlay is open and we are not in
            // binding state! only exception allowed is any CET bind
            const auto& overlayToggleModBind = Bindings::GetOverlayToggleModBind();
            const auto cetModBind = modBind.ModName == overlayToggleModBind.ModName;
            if (!cetModBind && (!m_cpVm->IsInitialized() || CET::Get().GetOverlay().IsEnabled()))
                return;

            // this handler is not for inputs! it should be used only on key up event for hotkeys!
            if (vkBind->IsInput())
                return;

            // execute CET binds immediately, otherwise cursor will not show on overlay toggle
            if (cetModBind)
                vkBind->Call(false);
            else
                m_queuedCallbacks.Add(vkBind->DelayedCall(false));
        }
    }
}

LRESULT VKBindings::HandleRAWInput(const HRAWINPUT achRAWInput)
{
    UINT dwSize{0};
    GetRawInputData(achRAWInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

    const auto lpb = std::make_unique<BYTE[]>(dwSize);
    if (GetRawInputData(achRAWInput, RID_INPUT, lpb.get(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
        Log::Warn("VKBindings::HandleRAWInput() - GetRawInputData() does not return correct size!");

    const auto* raw = reinterpret_cast<const RAWINPUT*>(lpb.get());
    if (raw->header.dwType == RIM_TYPEKEYBOARD)
    {
        const auto& kb = raw->data.keyboard;
        switch (kb.Message)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: return RecordKeyDown(kb.VKey);
        case WM_KEYUP:
        case WM_SYSKEYUP: return RecordKeyUp(kb.VKey);
        }
    }
    else if (raw->header.dwType == RIM_TYPEMOUSE)
    {
        if (m_isBindRecording && m_recordingModBind == Bindings::GetOverlayToggleModBind())
            return 0; // ignore mouse keys for toolbar key binding!

        const auto& m = raw->data.mouse;
        switch (m.usButtonFlags)
        {
        case RI_MOUSE_LEFT_BUTTON_DOWN: return RecordKeyDown(VK_LBUTTON);
        case RI_MOUSE_LEFT_BUTTON_UP: return RecordKeyUp(VK_LBUTTON);
        case RI_MOUSE_RIGHT_BUTTON_DOWN: return RecordKeyDown(VK_RBUTTON);
        case RI_MOUSE_RIGHT_BUTTON_UP: return RecordKeyUp(VK_RBUTTON);
        case RI_MOUSE_MIDDLE_BUTTON_DOWN: return RecordKeyDown(VK_MBUTTON);
        case RI_MOUSE_MIDDLE_BUTTON_UP: return RecordKeyUp(VK_MBUTTON);
        case RI_MOUSE_BUTTON_4_DOWN: return RecordKeyDown(VK_XBUTTON1);
        case RI_MOUSE_BUTTON_4_UP: return RecordKeyUp(VK_XBUTTON1);
        case RI_MOUSE_BUTTON_5_DOWN: return RecordKeyDown(VK_XBUTTON2);
        case RI_MOUSE_BUTTON_5_UP: return RecordKeyUp(VK_XBUTTON2);
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

void VKBindings::ClearRecording(const bool acClearBind)
{
    m_recordingWasKeyPressed = false;
    m_recording.fill(0);
    m_recordingLength = 0;
    m_isBindRecording = false;
    if (acClearBind)
    {
        m_recordingResult = 0;
        m_recordingModBind.ID.clear();
        m_recordingModBind.ModName.clear();
    }
}

void VKBindings::SetVM(const LuaVM* acpVm)
{
    // this should happen only once
    assert(m_cpVm == nullptr);
    // vm pointer shall be always valid when this is called!
    assert(acpVm != nullptr);

    m_cpVm = acpVm;
}
