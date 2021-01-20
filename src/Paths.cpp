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

Paths::Paths()
{
    TCHAR exePathBuf[MAX_PATH] = { 0 };
    GetModuleFileName(GetModuleHandle(nullptr), exePathBuf, std::size(exePathBuf));
    m_exe = exePathBuf;

    m_gameRoot = m_exe.parent_path();

    m_cetRoot = m_gameRoot;
    m_cetRoot /= "plugins";
    m_cetRoot /= "cyber_engine_tweaks";
    create_directories(m_cetRoot);
    
    m_config = m_cetRoot / "config.json";
    // remove empty config.json
    if (exists(m_config) && !file_size(m_config))
        std::filesystem::remove(m_config);

    m_vkBindings = m_cetRoot / "hotkeys.json";
    // remove empty vkbindings.json
    if (exists(m_vkBindings) && !file_size(m_vkBindings))
        std::filesystem::remove(m_vkBindings);

    m_modsRoot = m_cetRoot / "mods";
    create_directories(m_modsRoot);
}
