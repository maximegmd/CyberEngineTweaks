#pragma once

#include "Sandbox.h"

struct LuaSandbox
{
    LuaSandbox(Scripting* apScripting);
    ~LuaSandbox() = default;

    void Initialize();
    void ResetState();

    size_t CreateSandbox(const std::filesystem::path& acPath, bool aEnableExtraLibs = true, bool aEnableDB = true, bool aEnableIO = true);

    std::shared_ptr<spdlog::logger> InitializeLoggerForSandbox(Sandbox& aSandbox, const std::string& acName) const;
    
    sol::protected_function_result ExecuteFile(const std::string& acPath);
    sol::protected_function_result ExecuteString(const std::string& acString);
    
    Sandbox& operator[](size_t aID);
    const Sandbox& operator[](size_t aID) const;

    [[nodiscard]] TiltedPhoques::Locked<sol::state, std::recursive_mutex> GetState() const;
    
private:

    void InitializeExtraLibsForSandbox(Sandbox& aSandbox) const;
    void InitializeDBForSandbox(Sandbox& aSandbox) const;
    void InitializeIOForSandbox(Sandbox& aSandbox); 
    
    Scripting* m_pScripting;
    sol::environment m_env{ };
    std::vector<Sandbox> m_sandboxes{ };
    TiltedPhoques::Map<std::string, sol::object> m_modules{ };
};
