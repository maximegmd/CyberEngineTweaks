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

const std::filesystem::path& Paths::PersistentState() const
{
    return m_persistentState;
}

const std::filesystem::path& Paths::VKBindings() const
{
    return m_vkBindings;
}

const std::filesystem::path& Paths::ModsRoot() const
{
    return m_modsRoot;
}

const std::filesystem::path& Paths::R6CacheModdedRoot() const
{
    return m_r6CacheModdedRoot;
}

const std::filesystem::path& Paths::Fonts() const
{
    return m_fonts;
}

const std::filesystem::path& Paths::TweakDB() const
{
    return m_tweakdb;
}

Paths::Paths()
{
    TCHAR exePathBuf[MAX_PATH] = {0};
    GetModuleFileName(GetModuleHandle(nullptr), exePathBuf, static_cast<DWORD>(std::size(exePathBuf)));
    m_exe = exePathBuf;

    m_gameRoot = m_exe.parent_path();

    m_cetRoot = m_gameRoot;
#if GAME_CYBERPUNK
    m_cetRoot /= L"plugins";
    m_cetRoot /= L"cyber_engine_tweaks";
#else
    m_cetRoot /= L"red3ext";
    m_cetRoot /= L"cyber_engine_tweaks";
#endif
    create_directories(m_cetRoot);

    m_config = m_cetRoot / L"config.json";
    // remove empty config.json
    if (exists(m_config) && !file_size(m_config))
        std::filesystem::remove(m_config);

    m_persistentState = m_cetRoot / L"persistent.json";
    // remove empty persistentState.json
    if (exists(m_persistentState) && !file_size(m_persistentState))
        std::filesystem::remove(m_persistentState);

    m_vkBindings = m_cetRoot / L"bindings.json";
    // remove empty vkbindings.json
    if (exists(m_vkBindings) && !file_size(m_vkBindings))
        std::filesystem::remove(m_vkBindings);

    m_modsRoot = m_cetRoot / L"mods";
    create_directories(m_modsRoot);

    m_r6CacheModdedRoot = m_gameRoot;
    m_r6CacheModdedRoot /= L"..";
    m_r6CacheModdedRoot /= L"..";
    m_r6CacheModdedRoot /= L"r6";
    m_r6CacheModdedRoot /= L"cache";
    m_r6CacheModdedRoot /= L"modded";

    m_fonts = m_cetRoot / L"fonts";

    m_tweakdb = m_cetRoot / L"tweakdb";
}
