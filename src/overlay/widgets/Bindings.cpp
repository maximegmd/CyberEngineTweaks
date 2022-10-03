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
    const auto itemWidth = GetAlignedItemWidth(2);

    if (ImGui::Button("Save", ImVec2(itemWidth, 0)))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Reset changes", ImVec2(itemWidth, 0)))
        ResetChanges();

    ImGui::Spacing();

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


        if (ImGui::CollapsingHeader(activeModName.c_str()))
        {
            if (ImGui::BeginTable(("##" + activeModName).c_str(), 3, ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders))
            {

                for (auto& binding : modBindingsIt.value())
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    const VKModBind modBind{modBindingsIt.key(), binding.Bind.ID};

                    const auto onFinaizeBind = [this, &binding]{
                        auto codeBind = m_bindings.GetLastRecordingResult();
                        if (codeBind != binding.CodeBind)
                        {
                            if (m_bindings.IsFirstKeyUsed(codeBind))
                            {
                                // note - creating copy so we are not destroying reference to modBind when we unbind
                                const auto checkModBind = [this, &binding, codeBind](const VKModBind modBind) {
                                    const auto cetBind = modBind == s_overlayToggleModBind;
                                    if (!cetBind || binding.Bind.IsHotkey())
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
                                                if ((!cetBind && bindItCodeBindIsSame) || binding.Bind.IsInput() || bindIt->Bind.IsInput())
                                                {
                                                    m_bindings.UnBind(modBind);
                                                    bindIt->CodeBind = 0;
                                                }

                                                if (binding.Bind.IsInput() && bindIt->Bind.IsHotkey() && m_bindings.IsFirstKeyUsed(codeBind))
                                                {
                                                    bindIt->CodeBind = bindItCodeBind;
                                                    m_bindings.Bind(bindIt->CodeBind, modBind);
                                                }
                                                else
                                                    binding.CodeBind = codeBind;
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
                                binding.CodeBind = codeBind;
                        }
                    };

                    if (modBind == s_overlayToggleModBind)
                        UpdateAndDrawBinding(modBind, binding, onFinaizeBind, nullptr, 10.0f);
                    else
                    {
                        auto onUnBind = [this, &binding, &modBind] {
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

                ImGui::EndTable();
            }
        }
    }
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

        const auto onFinalizeBind = [this]{
            auto& bindings = CET::Get().GetBindings();

            s_overlayToggleBindInfo.CodeBind = bindings.GetLastRecordingResult();
            bindings.UnBind(s_overlayToggleBindInfo.CodeBind);

            m_vm.BlockDraw(false);
            ImGui::CloseCurrentPopup();
        };

        // TODO - do not hardcode offset! this somewhat works temporarily...
        UpdateAndDrawBinding(s_overlayToggleModBind, s_overlayToggleBindInfo, onFinalizeBind,nullptr, diffTextSz * 0.75f);

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
    const auto& allModsBinds = m_vm.GetAllBinds();

    if (!m_vkBindInfos.empty())
    {
        assert(m_vkBindInfos[s_overlayToggleModBind.ModName].size() == 1);
        const auto& overlayToggleBindInfo = m_vkBindInfos[s_overlayToggleModBind.ModName][0];
        s_overlayToggleBindInfo.CodeBind = overlayToggleBindInfo.CodeBind;
        s_overlayToggleBindInfo.SavedCodeBind = overlayToggleBindInfo.SavedCodeBind;
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
    }

    const bool bound = aVKBindInfo.CodeBind != 0;
    const bool modified = aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind;

    ImVec4 curTextColor { ImGui::GetStyleColorVec4(ImGuiCol_Text) };
    if (!bound)
        curTextColor = ImVec4(1.0f, modified ? 0.5f : 0.0f, 0.0f, 1.0f);
    else if (modified)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);

    const auto& bind = aVKBindInfo.Bind;

    ImGui::AlignTextToFramePadding();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + aOffsetX);

    ImGui::Button(bind.DisplayName.c_str(), ImVec2(-FLT_MIN, 0));
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !bind.Description.empty())
        ImGui::SetTooltip("%s", bind.Description.c_str());

    ImGui::TableNextColumn();

    const std::string vkStr { aVKBindInfo.IsBinding ? "BINDING..." : VKBindings::GetBindString(aVKBindInfo.CodeBind) };

    ImGui::PushID(&aVKBindInfo.CodeBind);
    if (ImGui::Button(vkStr.c_str(), ImVec2(-FLT_MIN, 0)))
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

    ImGui::TableNextColumn();

    ImGui::PushID(&aVKBindInfo.CodeBind);
    const bool unbindDisabled = !aUnBindCB || !aVKBindInfo.CodeBind;
    if (unbindDisabled)
        ImGui::BeginDisabled();
    if (ImGui::Button("UNBIND", ImVec2(-FLT_MIN, 0)))
    {
        m_bindings.StopRecordingBind();
        aVKBindInfo.IsBinding = false;

        aUnBindCB();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Unbind this binding.");
    if (unbindDisabled)
        ImGui::EndDisabled();
    ImGui::PopID();

    ImGui::PopStyleColor();

    m_madeChanges |= aVKBindInfo.IsBinding || aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind;
}
