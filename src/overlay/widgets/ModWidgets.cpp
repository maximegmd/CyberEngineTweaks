#include <stdafx.h>

#include "ModWidgets.h"

#include <scripting/LuaVM.h>

void ModWidgets::OnEnable()
{
    
}

void ModWidgets::OnDisable()
{
    
}

void ModWidgets::Update()
{    
    if (ImGui::Button("Reload All Mods"))
        LuaVM::Get().ReloadAllMods();

    ImGui::Spacing();
    ImGui::Text("Mod Widgets in construction! :P");
}
