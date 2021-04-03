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
        m_vm.BlockDraw(m_madeChanges);
        m_madeChanges = (HelperWidgets::UnsavedChangesPopup(m_openChangesModal, m_madeChanges, m_saveCB, m_loadCB) == 0);
        m_vm.BlockDraw(m_madeChanges);
        m_enabled = m_madeChanges;
    }
    if (!m_enabled)
    {
        // reset changes substates
        m_hotkeysChanged = false;
        m_inputsChanged = false;
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
    ImGui::SameLine();
    if (ImGui::Button("Reset changes"))
        ResetChanges();

    ImGui::Spacing();

    if (!m_luaVMReady && m_vm.IsInitialized())
        Load();
    
    if (ImGui::BeginTabBar("##BINDINGS", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip))
    {
        if (ImGui::BeginTabItem("Hotkeys"))
        {
            if (ImGui::BeginChild("##BINDINGS_HOTKEYS"))
                m_hotkeysChanged = DrawBindings(true);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Inputs"))
        {
            if (ImGui::BeginChild("##BINDINGS_INPUTS"))
                m_inputsChanged = DrawBindings(false);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        m_madeChanges = m_hotkeysChanged || m_inputsChanged;

        ImGui::EndTabBar();
    }    
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

bool Bindings::DrawBindings(bool aDrawHotkeys)
{
    bool madeChanges = false;

    if (m_luaVMReady)
    {
        const char* cpEmptyMessage
        {
            (aDrawHotkeys)
             ? ("This mod has no hotkeys, but it should have some inputs in other tab...")
             : ("This mod has no inputs, but it should have some hotkeys in other tab...")
        };

        std::string activeModName;
        std::string_view prevMod { "" };
        size_t modBindsForType { 0 };
        for (auto& vkBindInfo : m_vkBindInfos)
        {
            std::string_view curMod { vkBindInfo.Bind.ID };
            curMod = curMod.substr(0, curMod.find('.'));
            if (prevMod != curMod)
            {
                if (curMod == "cet")
                {
                    if (!aDrawHotkeys)
                        continue; // skip in this instance
                    activeModName = "Cyber Engine Tweaks";
                }
                else
                    activeModName = curMod;

                // transform to nicer format till modinfo is in
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
                        return static_cast<char>(std::toupper(static_cast<int>(c)));
                    }
                    return c;
                });

                // add vertical spacing when this is not first iteration and check if we drawn anything
                if (!prevMod.empty())
                {
                    if (!modBindsForType)
                    {
                        // we did not draw anything, write appropriate message so it is not empty
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
                        ImGui::TextUnformatted(cpEmptyMessage);
                    }
                    ImGui::Spacing();
                }

                ImGui::TextUnformatted(activeModName.c_str());
                ImGui::Spacing();

                prevMod = curMod;
            }

            if (aDrawHotkeys == vkBindInfo.Bind.IsHotkey())
            {
                madeChanges |= HelperWidgets::BindWidget(vkBindInfo, (vkBindInfo.Bind.ID != m_overlayKeyID), 10.0f);
                ++modBindsForType;
            } 
        }
    }
    else
        ImGui::TextUnformatted("LuaVM is not yet initialized!");

    return madeChanges;
};
