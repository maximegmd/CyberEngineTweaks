#include <stdafx.h>

#include "Bindings.h"
#include "overlay/Overlay.h"

#include <scripting/LuaVM.h>

Bindings::Bindings(VKBindings& aBindings, Overlay& aOverlay, LuaVM& aVm)
    : m_bindings(aBindings)
    , m_overlay(aOverlay)
    , m_vm(aVm)
    , m_overlayKeyID(m_overlay.GetBind().ID)
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
        m_vm.BlockUpdate(m_madeChanges);
        m_madeChanges = (HelperWidgets::UnsavedChangesPopup(m_openChangesModal, m_madeChanges, m_saveCB, m_loadCB) == 0);
        m_vm.BlockUpdate(m_madeChanges);
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
        if (!m_luaVMReady && m_vm.IsInitialized())
            Load();

        if (m_luaVMReady)
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

                    std::string curModStr{curMod};
                    ImGui::Text(curModStr.c_str());

                    prevMod = curMod;
                }

                m_madeChanges |= HelperWidgets::BindWidget(vkBindInfo, (vkBindInfo.Bind.ID != m_overlayKeyID));
            }
        }
        else
            ImGui::Text("LuaVM is not yet initialized!");
    }
    ImGui::EndChild();
    
}

void Bindings::Load()
{
    if (!m_vm.IsInitialized())
    {
        m_luaVMReady = false;
        return;
    }
    
    m_bindings.Load(m_overlay);
    m_vkBindInfos = m_bindings.InitializeMods(m_vm.GetBinds());
    m_luaVMReady = true;
}

void Bindings::Save()
{
    m_bindings.Save();

    if (!m_vm.IsInitialized())
    {
        m_luaVMReady = false;
        return;
    }
    
    m_vkBindInfos = m_bindings.InitializeMods(m_vm.GetBinds());
    m_luaVMReady = true;
}

void Bindings::ResetChanges()
{
    for (auto& vkBindInfo : m_vkBindInfos)
    {
        if (vkBindInfo.CodeBind == vkBindInfo.SavedCodeBind)
            continue;

        if (vkBindInfo.CodeBind)
            m_bindings.UnBind(vkBindInfo.CodeBind);
        if (vkBindInfo.SavedCodeBind)
            m_bindings.Bind(vkBindInfo.SavedCodeBind, vkBindInfo.Bind);
        vkBindInfo.CodeBind = vkBindInfo.SavedCodeBind;
    }
}
