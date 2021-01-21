#pragma once

#include "Widget.h"

struct Overlay;
struct LuaVM;

struct Hotkeys : Widget
{
    Hotkeys(VKBindings& aBindings, Overlay& aOverlay, LuaVM& aVm);
    ~Hotkeys() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
    
    void Load();
    void Save();

private:
    std::vector<VKBindInfo> m_vkBindInfos{ };
    VKBindings& m_bindings;
    Overlay& m_overlay;
    LuaVM& m_vm;
};
