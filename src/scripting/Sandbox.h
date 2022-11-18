#pragma once

struct Scripting;

struct Sandbox
{
    Sandbox(Scripting& aScripting, uint64_t aId, sol::table aBaseEnvironment, const std::filesystem::path& acRootPath);
    Sandbox(Sandbox&& aOther) noexcept;
    ~Sandbox() = default;

    Sandbox& operator=(Sandbox&& aOther) noexcept;

    sol::protected_function_result ExecuteFile(const std::string& acPath) const;
    sol::protected_function_result ExecuteString(const std::string& acString) const;

    uint64_t GetId() const;
    sol::environment& GetEnvironment();
    const sol::environment& GetEnvironment() const;
    const std::filesystem::path& GetRootPath() const;

private:

    Scripting& m_scripting;

    uint64_t m_id{0};
    sol::environment m_env;
    std::filesystem::path m_path{};
};