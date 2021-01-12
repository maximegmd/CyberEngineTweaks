#include <stdafx.h>

#include "Keybinds.h"

#include "HelperWidgets.h"

#include <scripting/LuaVM.h>

void Keybinds::OnEnable()
{
    Load();
    
    VKBindings::StopRecordingBind();
}

void Keybinds::OnDisable()
{
    VKBindings::StopRecordingBind();
}

void Keybinds::Update()
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

                BindWidget(vkBindInfo);
            }
        }
    }
    ImGui::EndChild();
    
}

void Keybinds::Load()
{
    auto& luaVM = LuaVM::Get();
    if (!luaVM.IsInitialized())
        return;

    auto& luaVMBinds = luaVM.GetBinds();
    
    VKBindings::Load();
    VKBindings::InitializeMods(luaVMBinds);

    m_vkBindInfos = luaVMBinds;
}

void Keybinds::Save()
{
    auto& luaVM = LuaVM::Get();
    if (!luaVM.IsInitialized())
        return;

    auto& luaVMBinds = luaVM.GetBinds();
    for (size_t i = 0; i < m_vkBindInfos.size(); ++i)
    {
        auto& vkBindInfo = m_vkBindInfos[i];
        if (vkBindInfo.CodeBind != vkBindInfo.SavedCodeBind)
            luaVMBinds[i].SavedCodeBind = vkBindInfo.Apply();
    }
    
    VKBindings::Save();
}
