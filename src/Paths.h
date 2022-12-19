#pragma once

struct Paths
{
    ~Paths() = default;

    const std::filesystem::path& Executable() const;
    const std::filesystem::path& GameRoot() const;
    const std::filesystem::path& CETRoot() const;
    const std::filesystem::path& Config() const;
    const std::filesystem::path& PersistentState() const;
    const std::filesystem::path& VKBindings() const;
    const std::filesystem::path& ModsRoot() const;
    const std::filesystem::path& R6CacheModdedRoot() const;
    const std::filesystem::path& Fonts() const;
    const std::filesystem::path& Theme() const;
    const std::filesystem::path& TweakDB() const;

private:
    friend struct CET;

    Paths();

    std::filesystem::path m_exe{};
    std::filesystem::path m_gameRoot{};
    std::filesystem::path m_cetRoot{};
    std::filesystem::path m_config{};
    std::filesystem::path m_persistentState{};
    std::filesystem::path m_vkBindings{};
    std::filesystem::path m_modsRoot{};
    std::filesystem::path m_r6CacheModdedRoot{};
    std::filesystem::path m_fonts{};
    std::filesystem::path m_theme{};
    std::filesystem::path m_tweakdb{};
};