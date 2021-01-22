#include <stdafx.h>

#include "Hotkeys.h"
#include "HelperWidgets.h"
#include "overlay/Overlay.h"


#include <scripting/LuaVM.h>

Hotkeys::Hotkeys(VKBindings& aBindings, Overlay& aOverlay, LuaVM& aVm)
    : m_bindings(aBindings)
    , m_overlay(aOverlay)
    , m_vm(aVm)
{
}

void Hotkeys::OnEnable()
{
    Load();
    
    m_bindings.StopRecordingBind();
}

void Hotkeys::OnDisable()
{
    m_bindings.StopRecordingBind();
}

void Hotkeys::Update()
{
    if (ImGui::Button("Load"))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();

    ImGui::Spacing();
    
    if (ImGui::BeginChild("##SETTINGS_ACTUAL", ImVec2(0,0), true))
    {
        if (m_vkBindInfos.empty())
            ImGui::Text("Looks empty here... Try to load some mod with bindings support!");
        else
        {
            std::string_view prevMod = "";
            int id = 0;
            for (auto& vkBindInfo : m_vkBindInfos)
            {
                std::string_view curMod = vkBindInfo.Bind.ID;
                curMod = curMod.substr(0,curMod.find('.'));
                if (prevMod != curMod)
                {
                    if (!prevMod.empty())
                        ImGui::Spacing();

                    std::string curModStr{ curMod };
                    ImGui::Text(curModStr.c_str());

                    prevMod = curMod;
                }

                HelperWidgets::BindWidget(vkBindInfo, m_overlay.GetBind().ID);
            }
        }
    }
    ImGui::EndChild();
    
}

void Hotkeys::Load()
{
    if (!m_vm.IsInitialized())
        return;
    
    m_bindings.Load(m_overlay);
    
    m_vkBindInfos = m_bindings.InitializeMods(m_vm.GetBinds());
}

void Hotkeys::Save()
{
    m_bindings.Save();

    if (!m_vm.IsInitialized())
        return;
    
    m_vkBindInfos = m_bindings.InitializeMods(m_vm.GetBinds());
}
