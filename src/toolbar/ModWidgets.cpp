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

    ImGui::Text("ModWidgets widget here!");
}
