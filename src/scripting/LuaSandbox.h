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

struct LuaSandbox
{
    LuaSandbox(sol::state_view aStateView);
    ~LuaSandbox() = default;

    void Initialize(sol::state_view aStateView);

    size_t CreateSandbox(const std::filesystem::path& acPath, bool aEnableExtraLibs = true, bool aEnableDB = true, bool aEnableIO = true);

    void ResetState();
    
    sol::protected_function_result ExecuteFile(const std::string& acPath);
    sol::protected_function_result ExecuteString(const std::string& acString);

    void InitializeExtraLibsForEnvironment(sol::environment& aEnvironment) const;
    
    void InitializeDBForEnvironment(sol::environment& aEnvironment, const std::filesystem::path& acRootPath) const;

    void InitializeIOForEnvironment(sol::environment& aEnvironment, const std::filesystem::path& acRootPath);

    std::shared_ptr<spdlog::logger> InitializeLoggerForEnvironment(sol::environment& aEnvironment, const std::filesystem::path& acPath, const std::string& acName) const;

    Sandbox& operator[](size_t id);
    const Sandbox& operator[](size_t id) const;
    
private:
    
    sol::state_view m_lua;
    sol::environment m_env{ };
    std::vector<Sandbox> m_sandboxes{ };
    TiltedPhoques::Map<std::string, sol::object> m_modules{ };
};