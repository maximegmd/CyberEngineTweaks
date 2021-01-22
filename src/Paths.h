#pragma once

struct Paths
{
    ~Paths() = default;

    const std::filesystem::path& Executable() const;
    const std::filesystem::path& GameRoot() const;
    const std::filesystem::path& CETRoot() const;
    const std::filesystem::path& Config() const;
    const std::filesystem::path& VKBindings() const;
    const std::filesystem::path& ModsRoot() const;

private:

    friend struct CET;

    Paths();
    
    std::filesystem::path m_exe{ };
    std::filesystem::path m_gameRoot{ };
    std::filesystem::path m_cetRoot{ };
    std::filesystem::path m_config{ };
    std::filesystem::path m_vkBindings{ };
    std::filesystem::path m_modsRoot{ };
};