#pragma once

#include "Sandbox.h"

struct LuaSandbox
{
    LuaSandbox(const Options& acOptions, Scripting& aScripting, const VKBindings& acVKBindings);
    ~LuaSandbox() = default;

    void Initialize();
    void PostInitializeScripting();
    void PostInitializeTweakDB();
    void PostInitializeMods();

    void ResetState();

    uint64_t CreateSandbox(const std::filesystem::path& acPath = "", const std::string& acName = "", bool aEnableExtraLibs = true, bool aEnableDB = true, bool aEnableIO = true, bool aEnableLogger = true);

    Sandbox& operator[](uint64_t aID);
    const Sandbox& operator[](uint64_t aID) const;

    [[nodiscard]] TiltedPhoques::Locked<sol::state, std::recursive_mutex> GetLockedState() const;

    void SetImGuiAvailable(bool aAvailable);
    bool GetImGuiAvailable() const;

    sol::table& GetGlobals();

private:

    void InitializeExtraLibsForSandbox(Sandbox& aSandbox, const sol::state& acpState) const;
    void InitializeDBForSandbox(Sandbox& aSandbox, const sol::state& acpState);
    void InitializeIOForSandbox(Sandbox& aSandbox, const sol::state& acpState, const std::string& acName);
    void InitializeLoggerForSandbox(Sandbox& aSandbox, const sol::state& acpState, const std::string& acName) const;

    void CloseDBForSandbox(const Sandbox& aSandbox) const;

    const Options& m_options;
    Scripting& m_scripting;
    const VKBindings& m_vkBindings;
    sol::table m_globals{};
    TiltedPhoques::Vector<Sandbox> m_sandboxes{};
    TiltedPhoques::Map<std::string, sol::object> m_modules{};

    bool m_imguiAvailable{ false };
};
