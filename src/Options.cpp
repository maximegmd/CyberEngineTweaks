#include <stdafx.h>

#include "Paths.h"
#include "Utils.h"

void Options::Load()
{
    IsFirstLaunch = !exists(m_paths.Config());
    if (!IsFirstLaunch)
    {
        std::ifstream configFile(m_paths.Config());
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
            
            // font config
            FontPath = config.value("font_path", FontPath);
            FontGlyphRanges = config.value("font_glyph_ranges", FontGlyphRanges);
            FontSize = config.value("font_size", FontSize);

            OverlayKeyBind = config.value("overlay_key", OverlayKeyBind);
            if (OverlayKeyBind == 0)
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

    config["overlay_key"] = OverlayKeyBind;
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
    config["font_path"] = FontPath;
    config["font_glyph_ranges"] = FontGlyphRanges;
    config["font_size"] = FontSize;

    std::ofstream o(m_paths.Config());
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

    Save();
}

Options::Options(Paths& aPaths)
    : m_paths(aPaths)
{
    const auto* exePathStr = aPaths.Executable().native().c_str(); 
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
    ExeValid = ExeValid || (aPaths.Executable().filename() == "Cyberpunk2077.exe");

    if (!ExeValid)
        throw std::runtime_error("Not Cyberpunk2077.exe");

    set_default_logger(CreateLogger(m_paths.CETRoot() / "cyber_engine_tweaks.log", "main"));

    spdlog::info("Cyber Engine Tweaks is starting...");

    GameImage.Initialize();

    if (GameImage.version)
    {
        spdlog::info("CET version {} [{}]", CET_BUILD_COMMIT, CET_BUILD_BRANCH);
        auto [major, minor] = GameImage.GetVersion();
        spdlog::info("Game version {}.{:02d}", major, minor);
        spdlog::info("Root path: \"{}\"", aPaths.GameRoot().string());
        spdlog::info("Cyber Engine Tweaks path: \"{}\"", aPaths.CETRoot().string());
        spdlog::info("Lua scripts search path: \"{}\"", aPaths.ModsRoot().string());

        if (GameImage.GetVersion() != GameImage.GetSupportedVersion())
        {
            auto [smajor, sminor] = GameImage.GetSupportedVersion();
            spdlog::error("Unsupported game version! Only {}.{:02d} is supported.", smajor, sminor);
            throw std::runtime_error("Unsupported version");
        }
            
    }
    else
    {
        spdlog::info("Unknown Game Version, update the mod");
        throw std::runtime_error("Unknown version");
    }

    Load();

    if (!IsFirstLaunch)
        Save();
}
