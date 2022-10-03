#include <stdafx.h>

#include "Bindings.h"

#include <CET.h>

namespace
{
    VKBind s_overlayToggleBind{ "overlay_key", "Overlay Key", "Use this hotkey to toggle overlay on and off.", []{ CET::Get().GetOverlay().Toggle(); }};
    VKBindInfo s_overlayToggleBindInfo{s_overlayToggleBind, 0, 0, false};
    VKModBind s_overlayToggleModBind{"cet", s_overlayToggleBind.ID};
}

bool VKBindInfo::operator==(const std::string& id) const
{
    return Bind.get() == id;
}

Bindings::Bindings(VKBindings& aBindings, LuaVM& aVm)
    : m_bindings(aBindings)
    , m_vm(aVm)
{
}

WidgetResult Bindings::OnEnable()
{
    if (!m_enabled)
    {
        m_bindings.StopRecordingBind();
        ResetChanges();
        m_enabled = true;
    }
    return m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

WidgetResult Bindings::OnDisable()
{
    WidgetResult result = WidgetResult::ENABLED;

    if (m_enabled)
    {
        m_vm.BlockDraw(m_madeChanges);
        const auto ret = HelperWidgets::UnsavedChangesPopup(
            m_openChangesModal,
            m_madeChanges,
            [this]{ Save(); },
            [this]{ ResetChanges(); });
        m_madeChanges = ret == HelperWidgets::THWUCPResult::CHANGED;
        m_vm.BlockDraw(m_madeChanges);

        m_enabled = m_madeChanges;
        if (ret == HelperWidgets::THWUCPResult::CANCEL)
        {
            CET::Get().GetOverlay().SetActiveWidget(WidgetID::BINDINGS);
            m_enabled = true;
            result = WidgetResult::CANCEL;
        }
    }

    if (!m_enabled)
        m_bindings.StopRecordingBind();

    if (result != WidgetResult::CANCEL)
        result = m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;

    return result;
}

void Bindings::Update()
{
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Reset changes"))
        ResetChanges();

    ImGui::Spacing();

    if (!m_luaVMReady && m_vm.IsInitialized())
        Initialize();

    if (!m_luaVMReady)
        ImGui::TextUnformatted("LuaVM is not yet initialized!");
    else
    {
        m_madeChanges = false;
        for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
        {
            // transform mod name to nicer format until modinfo is in
            std::string activeModName = modBindingsIt.key() == "cet" ? "Cyber Engine Tweaks" : modBindingsIt.key();
            bool capitalize = true;
            std::ranges::transform(std::as_const(activeModName), activeModName.begin(), [&capitalize](char c) {
                if (!std::isalnum(c))
                {
                    capitalize = true;
                    return ' ';
                }
                if (capitalize)
                {
                    capitalize = false;
                    return static_cast<char>(std::toupper(c));
                }
                return c;
            });

            ImGui::TextUnformatted(activeModName.c_str());
            ImGui::Spacing();

            for (auto& binding : modBindingsIt.value())
            {
                const VKModBind modBind{modBindingsIt.key(), binding.Bind.get().ID};

                const auto onFinaizeBind = [this, &binding, &modBind]{
                    auto codeBind = m_bindings.GetLastRecordingResult();
                    if (codeBind != binding.CodeBind && m_bindings.IsFirstKeyUsed(codeBind))
                    {
                        const auto checkModBind = [this, &binding, codeBind](const VKModBind& modBind, const std::function<void(const std::string&, VKBindInfo&)> onBindValid) {
                            if (modBind != s_overlayToggleModBind)
                            {
                                const auto& modName = modBind.ModName;
                                auto& bindInfos = m_vkBindInfos[modName];
                                const auto bindIt = std::find(bindInfos.begin(), bindInfos.end(), modBind.ID);
                                if (bindIt != bindInfos.cend())
                                {
                                    if (onBindValid)
                                        onBindValid(modName, *bindIt);
                                }
                                else
                                    binding.CodeBind = codeBind;
                            }
                        };

                        if (const auto* directModBind = m_bindings.GetModBindForBindCode(codeBind))
                        {
                            checkModBind(*directModBind, [this, &binding, codeBind](const std::string& modName, VKBindInfo& bindInfo) {
                                if (binding.Bind.get().IsHotkey() || bindInfo.Bind.get().IsInput())
                                {
                                    m_bindings.UnBind({modName, bindInfo.Bind.get().ID});
                                    bindInfo.CodeBind = 0;

                                    binding.CodeBind = codeBind;
                                }
                            });
                        }
                        else if (const auto* indirectModBind = m_bindings.GetModBindStartingWithBindCode(codeBind))
                        {
                            checkModBind(*indirectModBind, [&binding, codeBind](const std::string&, VKBindInfo& bindInfo) {
                                if (bindInfo.Bind.get().IsHotkey() == binding.Bind.get().IsHotkey())
                                    binding.CodeBind = codeBind;
                            });
                        }
                    }
                };

                const auto onUnBind = [this, &binding, &modBind] {
                    if (binding.IsBinding)
                    {
                        m_bindings.StopRecordingBind();
                        binding.IsBinding = false;
                    }
                    m_bindings.UnBind(modBind);
                    binding.CodeBind = 0;
                };

                UpdateAndDrawBinding(modBind, binding, onFinaizeBind, onUnBind, 10.0f);
            }
        }
    }
}

void Bindings::Save()
{
    if (!m_vm.IsInitialized())
    {
        m_luaVMReady = false;
        return;
    }

    for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
    {
        for (auto& binding : modBindingsIt.value())
        {
            if (binding.SavedCodeBind != binding.CodeBind)
                binding.SavedCodeBind = binding.CodeBind;
        }
    }

    m_bindings.Save();

    m_luaVMReady = true;
}

void Bindings::ResetChanges()
{
    const bool overlayToggleChanged = s_overlayToggleBindInfo.SavedCodeBind != s_overlayToggleBindInfo.CodeBind;

    if (overlayToggleChanged && s_overlayToggleBindInfo.CodeBind != 0)
        m_bindings.UnBind(s_overlayToggleBindInfo.CodeBind);

    for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
    {
        for (const auto& binding : modBindingsIt.value())
        {
            if (binding.CodeBind != 0 && binding.SavedCodeBind != binding.CodeBind)
                m_bindings.UnBind(binding.CodeBind);
        }
    }

    if (overlayToggleChanged && s_overlayToggleBindInfo.SavedCodeBind != 0)
    {
        m_bindings.Bind(s_overlayToggleBindInfo.SavedCodeBind, s_overlayToggleModBind);
        s_overlayToggleBindInfo.CodeBind = s_overlayToggleBindInfo.SavedCodeBind;
    }

    for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
    {
        for (auto& binding : modBindingsIt.value())
        {
            if (binding.SavedCodeBind != 0 && binding.SavedCodeBind != binding.CodeBind)
                m_bindings.Bind(binding.SavedCodeBind, {modBindingsIt.key(), binding.Bind.get().ID});

            binding.CodeBind = binding.SavedCodeBind;
        }
    }
}

bool Bindings::IsFirstTimeSetup()
{
    if (s_overlayToggleBindInfo.SavedCodeBind == 0)
    {
        s_overlayToggleBindInfo.CodeBind = CET::Get().GetBindings().GetBindCodeForModBind(s_overlayToggleModBind);
        s_overlayToggleBindInfo.SavedCodeBind = s_overlayToggleBindInfo.CodeBind;
    }
    return s_overlayToggleBindInfo.SavedCodeBind == 0;
}

bool Bindings::FirstTimeSetup()
{
    if (!IsFirstTimeSetup())
        return false;

    if (ImGui::BeginPopupModal("CET First Time Setup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        const auto shorterTextSz { ImGui::CalcTextSize("Combo can be composed from up to 4 keys.").x };
        const auto longerTextSz { ImGui::CalcTextSize("Please, bind some key combination for toggling overlay!").x };
        const auto diffTextSz { longerTextSz - shorterTextSz };

        ImGui::TextUnformatted("Please, bind some key combination for toggling overlay!");
        ImGui::SetCursorPosX(diffTextSz / 2);
        ImGui::TextUnformatted("Combo can be composed from up to 4 keys.");
        ImGui::Separator();

        const auto onFinaizeBind = [this]{
            auto& bindings = CET::Get().GetBindings();

            s_overlayToggleBindInfo.CodeBind = bindings.GetLastRecordingResult();
            bindings.UnBind(s_overlayToggleBindInfo.CodeBind);

            m_vm.BlockDraw(false);
            ImGui::CloseCurrentPopup();
        };

        // TODO - do not hardcode offset! this somewhat works temporarily...
        UpdateAndDrawBinding(s_overlayToggleModBind, s_overlayToggleBindInfo, onFinaizeBind,nullptr, diffTextSz * 0.75f);

        ImGui::EndPopup();

        if (!IsFirstTimeSetup())
        {
            m_bindings.Save();
            return false;
        }
        return true;
    }

    return false;
}

const VKModBind& Bindings::GetOverlayToggleModBind() noexcept
{
    return s_overlayToggleModBind;
}

const VKBind& Bindings::GetOverlayToggleBind() noexcept
{
    return s_overlayToggleBind;
}

void Bindings::Initialize()
{
    if (!m_vm.IsInitialized())
        return;

    const auto& allModsBinds = m_vm.GetAllBinds();

    m_vkBindInfos.clear();

    // emplace CET internal settings
    m_vkBindInfos[s_overlayToggleModBind.ModName].emplace_back(s_overlayToggleBindInfo);

    // emplace mod bindings
    for (const auto& modBindsIt : allModsBinds)
    {
        for (const auto& vkBind : modBindsIt.second.get())
        {
            auto& vkBindInfo = m_vkBindInfos[modBindsIt.first].emplace_back(vkBind);
            vkBindInfo.SavedCodeBind = m_bindings.GetBindCodeForModBind({modBindsIt.first, vkBind.ID});
            vkBindInfo.CodeBind = vkBindInfo.SavedCodeBind;
        }
    }

    m_luaVMReady = true;
}

void Bindings::UpdateAndDrawBinding(const VKModBind& acModBind, VKBindInfo& aVKBindInfo, TWidgetCB aFinalizeBindCB, TWidgetCB aUnBindCB, float aOffsetX)
{
    const auto isRecording = CET::Get().GetBindings().IsRecordingBind();
    if (aVKBindInfo.IsBinding && !isRecording)
    {
        const auto previousCodeBind = aVKBindInfo.CodeBind;

        if (aFinalizeBindCB)
            aFinalizeBindCB();

        if (previousCodeBind != aVKBindInfo.CodeBind)
            m_bindings.Bind(aVKBindInfo.CodeBind, acModBind);

        aVKBindInfo.IsBinding = false;
    };

    const bool bound = aVKBindInfo.CodeBind != 0;
    const bool modified = aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind;

    ImVec4 curTextColor { ImGui::GetStyleColorVec4(ImGuiCol_Text) };
    if (!bound)
        curTextColor = ImVec4(1.0f, modified ? 0.5f : 0.0f, 0.0f, 1.0f);
    else if (modified)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    const auto& bind = aVKBindInfo.Bind.get();
    std::string label { bind.IsHotkey() ? "[Hotkey] " : "[Input] " };
    label += bind.DisplayName;
    label += ':';

    ImGui::AlignTextToFramePadding();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + aOffsetX);

    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
    ImGui::TextUnformatted(label.c_str());
    ImGui::PopStyleColor();

    const std::string vkStr { aVKBindInfo.IsBinding ? "BINDING..." : VKBindings::GetBindString(aVKBindInfo.CodeBind) };

    ImGui::SameLine();
    ImGui::PushID(&aVKBindInfo.CodeBind);
    if (ImGui::Button(vkStr.c_str()))
    {
        if (!aVKBindInfo.IsBinding && !isRecording)
        {
            m_bindings.StartRecordingBind(acModBind);
            aVKBindInfo.IsBinding = true;
        }
    }
    ImGui::PopID();

    if (aUnBindCB && aVKBindInfo.CodeBind)
    {
        ImGui::SameLine();
        ImGui::PushID(&aVKBindInfo.SavedCodeBind);
        if (ImGui::Button("UNBIND"))
        {
            m_bindings.StopRecordingBind();
            aVKBindInfo.IsBinding = false;

            aUnBindCB();
        }
        ImGui::PopID();
    }

    m_madeChanges |= aVKBindInfo.IsBinding || aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind;
}
