#pragma once

struct Paths
{
    Paths() = delete;
    ~Paths() = delete;

    static void Initialize()
    {
        TCHAR exePathBuf[MAX_PATH] = { 0 };
        GetModuleFileName(GetModuleHandle(nullptr), exePathBuf, std::size(exePathBuf));
        ExePath = exePathBuf;

        RootPath = ExePath.parent_path();

        CETPath = RootPath;
        CETPath /= "plugins";
        CETPath /= "cyber_engine_tweaks";
        std::filesystem::create_directories(CETPath);

        ScriptsPath = CETPath / "mods";
        std::filesystem::create_directories(ScriptsPath);

        Initialized = true;
    }
    
    static inline std::filesystem::path ExePath{ };
    static inline std::filesystem::path RootPath{ };
    static inline std::filesystem::path CETPath{ };
    static inline std::filesystem::path ScriptsPath{ };

    static inline bool Initialized{ false };
};