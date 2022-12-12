#pragma once

#include "Widget.h"

struct VKBindInfo
{
    const VKBind& Bind;
    uint64_t CodeBind{0};
    uint64_t SavedCodeBind{0};
    bool IsBinding{false};

    bool operator==(const std::string& id) const;
};

struct LuaVM;
struct Bindings : Widget
{
    Bindings(VKBindings& aBindings, LuaVM& aVm);
    ~Bindings() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;

    void Save();
    void ResetChanges();

    [[nodiscard]] static bool IsFirstTimeSetup();
    [[nodiscard]] bool FirstTimeSetup();

    [[nodiscard]] static const VKModBind& GetOverlayToggleModBind() noexcept;
    [[nodiscard]] static const VKBind& GetOverlayToggleBind() noexcept;

protected:
    void OnUpdate() override;
    WidgetResult OnPopup() override;

private:
    void Initialize();
    void UpdateAndDrawBinding(const VKModBind& acModBind, VKBindInfo& aVKBindInfo);
    void UpdateAndDrawModBindings(const std::string& acModName, TiltedPhoques::Vector<VKBindInfo>& aVKBindInfos, size_t aHotkeyCount, bool aSimplified = false);

    TiltedPhoques::Map<std::string, std::pair<TiltedPhoques::Vector<VKBindInfo>, size_t>> m_vkBindInfos{};
    VKBindings& m_bindings;
    LuaVM& m_vm;

    TChangedCBResult m_popupResult{TChangedCBResult::APPLY};
    bool m_madeChanges{false};
    bool m_openChangesModal{true};
};
