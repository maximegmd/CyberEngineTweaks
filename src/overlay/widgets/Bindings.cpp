#include <stdafx.h>

#include "Bindings.h"

#include <CET.h>
#include <Utils.h>

namespace
{
    VKBind s_overlayToggleBind{ "overlay_key", "Overlay Key", "Use this hotkey to toggle overlay on and off.", [] {
        if (!CET::Get().GetBindings().IsRecordingBind())
            CET::Get().GetOverlay().Toggle();
    }};
    VKBindInfo s_overlayToggleBindInfo{s_overlayToggleBind, 0, 0, false};
    VKModBind s_overlayToggleModBind{"cet", s_overlayToggleBind.ID};
}

bool VKBindInfo::operator==(const std::string& id) const
{
    return Bind == id;
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
        Initialize();
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
        const auto ret = UnsavedChangesPopup(
            m_openChangesModal,
            m_madeChanges,
            [this]{ Save(); },
            [this]{ ResetChanges(); });
        m_madeChanges = ret == THWUCPResult::CHANGED;
        m_vm.BlockDraw(m_madeChanges);

        m_enabled = m_madeChanges;
        if (ret == THWUCPResult::CANCEL)
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
    const auto frameSize = ImVec2(ImGui::GetContentRegionAvailWidth(), -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().FramePadding.y + 2.0f));
    if (ImGui::BeginChildFrame(ImGui::GetID("Bindings"), frameSize))
    {
        m_madeChanges = false;
        for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
            UpdateAndDrawModBindings(modBindingsIt.key(), modBindingsIt.value());
    }
    ImGui::EndChildFrame();

    ImGui::Spacing();

    const auto itemWidth = GetAlignedItemWidth(2);
    if (ImGui::Button("Save", ImVec2(itemWidth, 0)))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Reset changes", ImVec2(itemWidth, 0)))
        ResetChanges();
}

void Bindings::Save()
{
    for (auto modBindingsIt = m_vkBindInfos.begin(); modBindingsIt != m_vkBindInfos.end(); ++modBindingsIt)
    {
        for (auto& binding : modBindingsIt.value())
        {
            if (binding.SavedCodeBind != binding.CodeBind)
                binding.SavedCodeBind = binding.CodeBind;
        }
    }

    m_bindings.Save();
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
                m_bindings.Bind(binding.SavedCodeBind, {modBindingsIt.key(), binding.Bind.ID});

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

        auto& cetBinds = m_vkBindInfos.at(s_overlayToggleModBind.ModName);
        UpdateAndDrawModBindings(s_overlayToggleModBind.ModName, cetBinds, true);

        auto& cetOverlayToggle = cetBinds[0];
        if (cetOverlayToggle.CodeBind != 0)
        {
            cetOverlayToggle.SavedCodeBind = cetOverlayToggle.CodeBind;

            m_vm.BlockDraw(false);
            m_bindings.Save();

            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
    }

    return true;
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
    const auto& allModsBinds = m_vm.GetAllBinds();

    if (!m_vkBindInfos.empty())
    {
        assert(m_vkBindInfos.at(s_overlayToggleModBind.ModName).size() == 1);
        const auto& overlayToggleBindInfo = m_vkBindInfos.at(s_overlayToggleModBind.ModName)[0];

        s_overlayToggleBindInfo.CodeBind = overlayToggleBindInfo.CodeBind;
        s_overlayToggleBindInfo.SavedCodeBind = overlayToggleBindInfo.SavedCodeBind;

        if (s_overlayToggleBindInfo.SavedCodeBind == 0)
        {
            assert(s_overlayToggleBindInfo.CodeBind);
            s_overlayToggleBindInfo.SavedCodeBind = s_overlayToggleBindInfo.CodeBind;
        }

        m_vkBindInfos.clear();
    }

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

    // we have only one binding! if there are more for some reason, something didnt work as expected...
    assert(m_vkBindInfos[s_overlayToggleModBind.ModName].size() == 1);
}

void Bindings::UpdateAndDrawBinding(const VKModBind& acModBind, VKBindInfo& aVKBindInfo)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    const auto isRecording = m_bindings.IsRecordingBind();
    if (aVKBindInfo.IsBinding && !isRecording)
    {
        const auto previousCodeBind = aVKBindInfo.CodeBind;

        auto codeBind = m_bindings.GetLastRecordingResult();
        if (codeBind != aVKBindInfo.CodeBind)
        {
            if (m_bindings.IsFirstKeyUsed(codeBind))
            {
                // note - creating copy so we are not destroying reference to modBind when we unbind
                const auto checkModBind = [this, &aVKBindInfo, codeBind](const VKModBind modBind) {
                    const auto cetBind = modBind == s_overlayToggleModBind;
                    if (!cetBind || aVKBindInfo.Bind.IsHotkey())
                    {
                        const auto& modName = modBind.ModName;
                        auto& bindInfos = m_vkBindInfos[modName];
                        const auto bindIt = std::find(bindInfos.begin(), bindInfos.end(), modBind.ID);
                        if (bindIt != bindInfos.cend())
                        {
                            const auto bindItCodeBindIsSame = bindIt->CodeBind == codeBind;
                            if (!cetBind || !bindItCodeBindIsSame)
                            {
                                const auto bindItCodeBind = bindIt->CodeBind;
                                if ((!cetBind && bindItCodeBindIsSame) || aVKBindInfo.Bind.IsInput() || bindIt->Bind.IsInput())
                                {
                                    m_bindings.UnBind(modBind);
                                    bindIt->CodeBind = 0;
                                }

                                if (aVKBindInfo.Bind.IsInput() && bindIt->Bind.IsHotkey() && m_bindings.IsFirstKeyUsed(codeBind))
                                {
                                    bindIt->CodeBind = bindItCodeBind;
                                    m_bindings.Bind(bindIt->CodeBind, modBind);
                                }
                                else
                                    aVKBindInfo.CodeBind = codeBind;
                            }
                        }
                    }
                };

                if (const auto* directModBind = m_bindings.GetModBindForBindCode(codeBind))
                    checkModBind(*directModBind);
                else if (const auto* indirectModBind = m_bindings.GetModBindStartingWithBindCode(codeBind))
                    checkModBind(*indirectModBind);
            }
            else
                aVKBindInfo.CodeBind = codeBind;
        }

        if (previousCodeBind != aVKBindInfo.CodeBind)
            m_bindings.Bind(aVKBindInfo.CodeBind, acModBind);

        aVKBindInfo.IsBinding = false;
    }

    bool bound = aVKBindInfo.CodeBind != 0;
    const bool unbindable = bound && acModBind != s_overlayToggleModBind;
    const bool modified = aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind;

    ImVec4 curTextColor { ImGui::GetStyleColorVec4(ImGuiCol_Text) };
    if (!bound)
        curTextColor = ImVec4(1.0f, modified ? 0.5f : 0.0f, 0.0f, 1.0f);
    else if (modified)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);

    const auto& bind = aVKBindInfo.Bind;

    ImGui::Button(bind.DisplayName.c_str(), ImVec2(-FLT_MIN, 0));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !bind.Description.empty())
        ImGui::SetTooltip("%s", bind.Description.c_str());

    ImGui::TableNextColumn();

    const auto currentBindState = aVKBindInfo.IsBinding ? m_bindings.GetLastRecordingResult() : aVKBindInfo.CodeBind;
    ImGui::PushID(&aVKBindInfo.CodeBind);
    if (ImGui::Button(VKBindings::GetBindString(currentBindState).c_str(), ImVec2(unbindable ? -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x) : -FLT_MIN, 0)))
    {
        if (!aVKBindInfo.IsBinding && !isRecording)
        {
            m_bindings.StartRecordingBind(acModBind);
            aVKBindInfo.IsBinding = true;
        }
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", bind.IsHotkey() ? "Bind up to 4 key combination to this binding." : "Bind single input to this binding.");
    ImGui::PopID();

    if (unbindable)
    {
        ImGui::SameLine();

        ImGui::PushID(&aVKBindInfo.SavedCodeBind);
        if (ImGui::Checkbox("##IsBound", &bound))
        {
            if (aVKBindInfo.CodeBind != 0)
            {
                m_bindings.StopRecordingBind();
                aVKBindInfo.IsBinding = false;

                m_bindings.UnBind(acModBind);
                aVKBindInfo.CodeBind = 0;
            }
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Uncheck this checkbox to unbind this binding.");
        ImGui::PopID();
    }

    ImGui::PopStyleColor();

    m_madeChanges |= aVKBindInfo.IsBinding || aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind;
}

void Bindings::UpdateAndDrawModBindings(const std::string& acModName, TiltedPhoques::Vector<VKBindInfo>& acVKBindInfos, bool aSimplified)
{
    // transform mod name to nicer format until modinfo is in
    std::string activeModName = acModName == s_overlayToggleModBind.ModName ? "Cyber Engine Tweaks" : acModName;
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

    auto renderedHeader = aSimplified;
    if (!renderedHeader)
        renderedHeader = ImGui::CollapsingHeader(activeModName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (renderedHeader)
    {
        if (ImGui::BeginTable(("##" + activeModName).c_str(), 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders))
        {
            for (auto& binding : acVKBindInfos)
                UpdateAndDrawBinding({acModName, binding.Bind.ID}, binding);

            ImGui::EndTable();
        }
    }
}
