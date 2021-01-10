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
    ImGui::Text("ModWidgets widget here!");

    if (ImGui::Button("Reload All Mods"))
        LuaVM::Get().ReloadAllMods();
}
