#include <stdafx.h>

const std::filesystem::path& Paths::Executable() const
{
    return m_exe;
}

const std::filesystem::path& Paths::GameRoot() const
{
    return m_gameRoot;
}

const std::filesystem::path& Paths::CETRoot() const
{
    return m_cetRoot;
}

const std::filesystem::path& Paths::Config() const
{
    return m_config;
}

const std::filesystem::path& Paths::VKBindings() const
{
    return m_vkBindings;
}

const std::filesystem::path& Paths::ModsRoot() const
{
    return m_modsRoot;
}

const std::filesystem::path& Paths::REDmodsRoot() const
{
    return m_REDmodsRoot;
}

const std::filesystem::path& Paths::ArchiveModsRoot() const 
{ 
    return m_archiveModsRoot; 
}

const std::filesystem::path& Paths::R6CacheModdedRoot() const
{
    return m_r6CacheModdedRoot;
}

Paths::Paths()
{
    TCHAR exePathBuf[MAX_PATH] = { 0 };
    GetModuleFileName(GetModuleHandle(nullptr), exePathBuf, std::size(exePathBuf));
    m_exe = exePathBuf;

    m_gameRoot = m_exe.parent_path();

    m_cetRoot = m_gameRoot;
    m_cetRoot /= L"plugins";
    m_cetRoot /= L"cyber_engine_tweaks";
    create_directories(m_cetRoot);

    m_config = m_cetRoot / L"config.json";
    // remove empty config.json
    if (exists(m_config) && !file_size(m_config))
        std::filesystem::remove(m_config);

    m_vkBindings = m_cetRoot / L"bindings.json";
    // remove empty vkbindings.json
    if (exists(m_vkBindings) && !file_size(m_vkBindings))
        std::filesystem::remove(m_vkBindings);

    m_modsRoot = m_cetRoot / L"mods";
    create_directories(m_modsRoot);

    m_REDmodsRoot = m_gameRoot;
    m_REDmodsRoot /= "..";
    m_REDmodsRoot /= "..";
    m_REDmodsRoot /= "mods";

    m_archiveModsRoot = m_gameRoot;
    m_archiveModsRoot /= L"..";
    m_archiveModsRoot /= L"..";
    m_archiveModsRoot /= L"archive";
    m_archiveModsRoot /= L"pc";
    m_archiveModsRoot /= L"mod";

    m_r6CacheModdedRoot = m_gameRoot;
    m_r6CacheModdedRoot /= L"..";
    m_r6CacheModdedRoot /= L"..";
    m_r6CacheModdedRoot /= L"r6";
    m_r6CacheModdedRoot /= L"cache";
    m_r6CacheModdedRoot /= L"modded";
}
