#include <stdafx.h>

void Options::Initialize()
{
    if (!Paths::Initialized)
        return;

    const auto* exePathStr = Paths::ExePath.native().c_str(); 
    int verInfoSz = GetFileVersionInfoSize(exePathStr, nullptr);
    if(verInfoSz) 
    {
        auto verInfo = std::make_unique<BYTE[]>(verInfoSz);
        if(GetFileVersionInfo(exePathStr, 0, verInfoSz, verInfo.get())) 
        {
            struct 
            {
                WORD Language;
                WORD CodePage;
            } *pTranslations;

            UINT transBytes = 0;
            if(VerQueryValue(verInfo.get(), _T("\\VarFileInfo\\Translation"), reinterpret_cast<void**>(&pTranslations), &transBytes)) 
            {
                UINT dummy;
                TCHAR* productName = nullptr;
                TCHAR subBlock[64];
                for(UINT i = 0; i < (transBytes / sizeof(*pTranslations)); i++)
                {
                    _stprintf(subBlock, _T("\\StringFileInfo\\%04x%04x\\ProductName"), pTranslations[i].Language, pTranslations[i].CodePage);
                    if(VerQueryValue(verInfo.get(), subBlock, reinterpret_cast<void**>(&productName), &dummy)) 
                        if (_tcscmp(productName, _T("Cyberpunk 2077")) == 0) 
                        {
                            ExeValid = true;
                            break;
                        }
                }
            }
        }
    }
    // check if exe name matches in case previous check fails
    ExeValid = ExeValid || (Paths::ExePath.filename() == "Cyberpunk2077.exe");

    if (!ExeValid)
        return;

    Logger::InfoToMain("Cyber Engine Tweaks is starting...");

    GameImage.Initialize();

    if (GameImage.version)
    {
        auto [major, minor] = GameImage.GetVersion();
        Logger::InfoToMainFmt("Game version {}.{:02d}", major, minor);
        Logger::InfoToMainFmt("Root path: \"{}\"", Paths::RootPath.string());
        Logger::InfoToMainFmt("Cyber Engine Tweaks path: \"{}\"", Paths::CETPath.string());
        Logger::InfoToMainFmt("Lua scripts search path: \"{}\"", Paths::ModsPath.string());
    }
    else
        Logger::InfoToMain("Unknown Game Version, update the mod");

    Load();

    if (!IsFirstLaunch)
        Save();

    Initialized = true;
}

void Options::Load()
{
    IsFirstLaunch = !std::filesystem::exists(Paths::ConfigPath);
    if (!IsFirstLaunch)
    {
        std::ifstream configFile(Paths::ConfigPath);
        if(configFile)
        {
            auto config = nlohmann::json::parse(configFile);
            PatchEnableDebug = config.value("enable_debug", PatchEnableDebug);
            PatchRemovePedestrians = config.value("remove_pedestrians", PatchRemovePedestrians);
            PatchSkipStartMenu = config.value("skip_start_menu", PatchSkipStartMenu);
            PatchAsyncCompute = config.value("disable_async_compute", PatchAsyncCompute);
            PatchAntialiasing = config.value("disable_antialiasing", PatchAntialiasing);
            PatchDisableIntroMovies = config.value("disable_intro_movies", PatchDisableIntroMovies);
            PatchDisableVignette = config.value("disable_vignette", PatchDisableVignette);
            PatchDisableBoundaryTeleport = config.value("disable_boundary_teleport", PatchDisableBoundaryTeleport);
            PatchDisableWin7Vsync = config.value("disable_win7_vsync", PatchDisableWin7Vsync);

            DumpGameOptions = config.value("dump_game_options", DumpGameOptions);
            ToolbarKey = config.value("toolbar_key", ToolbarKey);
            if (ToolbarKey != 0)
                ToolbarChar = MapVirtualKey(ToolbarKey, MAPVK_VK_TO_CHAR);
            else
                IsFirstLaunch = true; // is for sure in this case

            // check old config names
            if (config.value("unlock_menu", false))
                PatchEnableDebug = true;
        }
        configFile.close();
    }
}

void Options::Save()
{
    nlohmann::json config;

    config["toolbar_key"] = ToolbarKey;
    config["enable_debug"] = PatchEnableDebug;
    config["remove_pedestrians"] = PatchRemovePedestrians;
    config["disable_async_compute"] = PatchAsyncCompute;
    config["disable_antialiasing"] = PatchAntialiasing;
    config["skip_start_menu"] = PatchSkipStartMenu;
    config["disable_intro_movies"] = PatchDisableIntroMovies;
    config["disable_vignette"] = PatchDisableVignette;
    config["disable_boundary_teleport"] = PatchDisableBoundaryTeleport;
    config["disable_win7_vsync"] = PatchDisableWin7Vsync;
    config["dump_game_options"] = DumpGameOptions;

    std::ofstream o(Paths::ConfigPath);
    o << config.dump(4) << std::endl;
}

void Options::ResetToDefaults()
{
    PatchEnableDebug = false;
    PatchRemovePedestrians = false;
    PatchAsyncCompute = false;
    PatchAntialiasing = false;
    PatchSkipStartMenu = false;
    PatchDisableIntroMovies = false;
    PatchDisableVignette = false;
    PatchDisableBoundaryTeleport = false;
    PatchDisableWin7Vsync = false;
    DumpGameOptions = false;
}
