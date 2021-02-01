#pragma once

struct Sandbox
{
    Sandbox(sol::state_view aStateView, sol::environment aBaseEnvironment, const std::filesystem::path& acRootPath);
    ~Sandbox() = default;
    
    sol::protected_function_result ExecuteFile(const std::string& acPath);
    sol::protected_function_result ExecuteString(const std::string& acString);
    
    sol::state_view& GetStateView();
    sol::environment& GetEnvironment();
    const std::filesystem::path& GetRootPath() const;
    
private:
    sol::state_view m_lua;
    sol::environment m_env;
    std::filesystem::path m_path{ };
};