#pragma once

#include "Widget.h"
#include "HelperWidgets.h"

struct Overlay;
struct LuaVM;

struct VKBindInfo
{
    std::reference_wrapper<const VKBind> Bind;
    uint64_t CodeBind{ 0 };
    uint64_t SavedCodeBind{ 0 };
    bool IsBinding{ false };

    bool operator==(const std::string& id) const;
};

struct Bindings : Widget
{
    Bindings(VKBindings& aBindings, LuaVM& aVm);
    ~Bindings() override = default;

    bool OnEnable() override;
    bool OnDisable() override;
    void Update() override;

    void Save();
    void ResetChanges();

    [[nodiscard]] static bool IsFirstTimeSetup();
    [[nodiscard]] bool FirstTimeSetup();

    [[nodiscard]] static const VKModBind& GetOverlayToggleModBind() noexcept;
    [[nodiscard]] static const VKBind& GetOverlayToggleBind() noexcept;

private:
    void Initialize();
    void UpdateAndDrawBinding(const VKModBind& acModBind, VKBindInfo& aVKBindInfo, TWidgetCB aFinalizeBindCB, TWidgetCB aUnBindCB, float aOffsetX);

    TiltedPhoques::Map<std::string, TiltedPhoques::Vector<VKBindInfo>> m_vkBindInfos{ };
    VKBindings& m_bindings;
    LuaVM& m_vm;

    bool m_luaVMReady{ false };
    bool m_enabled{ false };
    bool m_madeChanges{ false };
    bool m_openChangesModal{ true };
};
