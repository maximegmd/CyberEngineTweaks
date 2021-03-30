#include <stdafx.h>

#include "Bindings.h"
#include "HelperWidgets.h"
#include "overlay/Overlay.h"

#include <scripting/LuaVM.h>

Bindings::Bindings(VKBindings& aBindings, Overlay& aOverlay, LuaVM& aVm)
    : m_bindings(aBindings)
    , m_overlay(aOverlay)
    , m_vm(aVm)
{
}

bool Bindings::OnEnable()
{
    if (!m_enabled)
    {
        m_bindings.StopRecordingBind();
        Load();
        m_enabled = true;
    }
    return m_enabled;
}

bool Bindings::OnDisable()
{
    if (m_enabled)
    {
        m_bindings.StopRecordingBind();

        if (m_madeChanges)
        {
            if (!m_showChangesModal)
            {
                ImGui::OpenPopup("Unapplied changes");
                m_showChangesModal = true;
            }

            if (ImGui::BeginPopupModal("Unapplied changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("You have some unsaved changes.\nDo you wish to apply them or discard them?");
                ImGui::Separator();

                if (ImGui::Button("Apply", ImVec2(120, 0)))
                {
                    Save();
                    m_showChangesModal = false;
                    m_madeChanges = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Discard", ImVec2(120, 0)))
                {
                    Load();
                    m_showChangesModal = false;
                    m_madeChanges = false;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();

                ImGui::EndPopup();
            }
        }

        m_enabled = m_madeChanges;
    }
    return !m_enabled;
}

void Bindings::Update()
{
    if (ImGui::Button("Load"))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();

    ImGui::Spacing();
    
    if (ImGui::BeginChild("##BINDINGS"))
    {
        // reset dirty state
        m_madeChanges = false;

        std::string_view prevMod = "";
        for (auto& vkBindInfo : m_vkBindInfos)
        {
            std::string_view curMod = vkBindInfo.Bind.ID;
            curMod = curMod.substr(0, curMod.find('.'));
            if (prevMod != curMod)
            {
                if (!prevMod.empty())
                    ImGui::Spacing();

                std::string curModStr { curMod };
                ImGui::Text(curModStr.c_str());

                prevMod = curMod;
            }

            m_madeChanges |= HelperWidgets::BindWidget(vkBindInfo, m_overlay.GetBind().ID);
        }
    }
    ImGui::EndChild();
    
}

void Bindings::Load()
{
    if (!m_vm.IsInitialized())
        return;
    
    m_bindings.Load(m_overlay);
    
    m_vkBindInfos = m_bindings.InitializeMods(m_vm.GetBinds());
}

void Bindings::Save()
{
    m_bindings.Save();

    if (!m_vm.IsInitialized())
        return;
    
    m_vkBindInfos = m_bindings.InitializeMods(m_vm.GetBinds());
}
