#pragma once

#include "Sandbox.h"

struct LuaSandbox
{
    LuaSandbox(Scripting* apScripting, const VKBindings& acVKBindings);
    ~LuaSandbox() = default;

    void Initialize();
    void PostInitialize() const;
    void ResetState();

    uint64_t CreateSandbox(const std::filesystem::path& acPath = "", const std::string& acName = "", bool aEnableImGui = true, bool aEnableExtraLibs = true, bool aEnableDB = true, bool aEnableIO = true, bool aEnableLogger = true);

    sol::protected_function_result ExecuteFile(const std::string& acPath) const;
    sol::protected_function_result ExecuteString(const std::string& acString) const;

    Sandbox& operator[](uint64_t aID);
    const Sandbox& operator[](uint64_t aID) const;

    [[nodiscard]] TiltedPhoques::Locked<sol::state, std::recursive_mutex> GetLockedState() const;

private:

    void InitializeImGuiForSandbox(Sandbox& aSandbox, const sol::state& acpState) const;
    void InitializeExtraLibsForSandbox(Sandbox& aSandbox, const sol::state& acpState) const;
    void InitializeDBForSandbox(Sandbox& aSandbox, const sol::state& acpState);
    void InitializeIOForSandbox(Sandbox& aSandbox, const sol::state& acpState, const std::string& acName);
    void InitializeLoggerForSandbox(Sandbox& aSandbox, const sol::state& acpState, const std::string& acName) const;

    void CloseDBForSandbox(const Sandbox& aSandbox) const;

    Scripting* m_pScripting;
    const VKBindings& m_vkBindings;
    sol::environment m_env{};
    TiltedPhoques::Vector<Sandbox> m_sandboxes{};
    TiltedPhoques::Map<std::string, sol::object> m_modules{};
};
