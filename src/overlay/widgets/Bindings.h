#pragma once

#include "Widget.h"
#include "HelperWidgets.h"

struct Overlay;
struct LuaVM;

struct Bindings : Widget
{
    Bindings(VKBindings& aBindings, Overlay& aOverlay, LuaVM& aVm);
    ~Bindings() override = default;

    bool OnEnable() override;
    bool OnDisable() override;
    void Update() override;
    
    void Load();
    void Save();

private:
    std::vector<VKBindInfo> m_vkBindInfos{ };
    VKBindings& m_bindings;
    Overlay& m_overlay;
    LuaVM& m_vm;

    std::string m_overlayKeyID;

    HelperWidgets::TUCHPSave m_saveCB { [this](){ Save(); } };
    HelperWidgets::TUCHPLoad m_loadCB { [this](){ Load(); } };

    bool m_luaVMReady{ false };
    bool m_enabled{ false };
    bool m_madeChanges{ false };
    bool m_openChangesModal{ true };
};
