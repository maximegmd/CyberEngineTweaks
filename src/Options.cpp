#include <stdafx.h>

#include "Paths.h"
#include "Utils.h"

void Options::Load()
{
    const auto path = GetAbsolutePath(m_paths.Config(), "", false);
    if (!path.empty())
    {
        std::ifstream configFile(path);
        if(configFile)
        {
            auto config = nlohmann::json::parse(configFile);
            PatchRemovePedestrians = config.value("remove_pedestrians", PatchRemovePedestrians);
            PatchSkipStartMenu = config.value("skip_start_menu", PatchSkipStartMenu);
            PatchAmdSmt = config.value("amd_smt", PatchAmdSmt);
            PatchAsyncCompute = config.value("disable_async_compute", PatchAsyncCompute);
            PatchAntialiasing = config.value("disable_antialiasing", PatchAntialiasing);
            PatchDisableIntroMovies = config.value("disable_intro_movies", PatchDisableIntroMovies);
            PatchDisableVignette = config.value("disable_vignette", PatchDisableVignette);
            PatchDisableBoundaryTeleport = config.value("disable_boundary_teleport", PatchDisableBoundaryTeleport);
            PatchDisableWin7Vsync = config.value("disable_win7_vsync", PatchDisableWin7Vsync);
            PatchMinimapFlicker = config.value("minimap_flicker", PatchMinimapFlicker);

            RemoveDeadBindings = config.value("cetdev_remove_dead_bindings", RemoveDeadBindings);
            EnableImGuiAssertions = config.value("cetdev_enable_imgui_assertions", EnableImGuiAssertions);
            DumpGameOptions = config.value("dump_game_options", DumpGameOptions);
            PatchEnableDebug = config.value("enable_debug", PatchEnableDebug);

            // font config
            FontPath = config.value("font_path", FontPath);
            FontGlyphRanges = config.value("font_glyph_ranges", FontGlyphRanges);
            FontSize = config.value("font_size", FontSize);

            // check old config names
            if (config.value("unlock_menu", false))
                PatchEnableDebug = true;
        }
        configFile.close();
    }

    // set global "Enable ImGui Assertions"
    g_ImGuiAssertionsEnabled = EnableImGuiAssertions;
}

void Options::Save()
{
    nlohmann::json config;

    config["remove_pedestrians"] = PatchRemovePedestrians;
    config["disable_async_compute"] = PatchAsyncCompute;
    config["disable_antialiasing"] = PatchAntialiasing;
    config["skip_start_menu"] = PatchSkipStartMenu;
    config["amd_smt"] = PatchAmdSmt;
    config["disable_intro_movies"] = PatchDisableIntroMovies;
    config["disable_vignette"] = PatchDisableVignette;
    config["disable_boundary_teleport"] = PatchDisableBoundaryTeleport;
    config["disable_win7_vsync"] = PatchDisableWin7Vsync;
    config["minimap_flicker"] = PatchMinimapFlicker;

    config["cetdev_remove_dead_bindings"] = RemoveDeadBindings;
    config["cetdev_enable_imgui_assertions"] = EnableImGuiAssertions;
    config["enable_debug"] = PatchEnableDebug;
    config["dump_game_options"] = DumpGameOptions;

    config["font_path"] = FontPath;
    config["font_glyph_ranges"] = FontGlyphRanges;
    config["font_size"] = FontSize;

    const auto path = GetAbsolutePath(m_paths.Config(), "", true);
    std::ofstream o(path);
    o << config.dump(4) << std::endl;

    // set global "Enable ImGui Assertions"
    g_ImGuiAssertionsEnabled = EnableImGuiAssertions;
}

void Options::ResetToDefaults()
{
    PatchRemovePedestrians = false;
    PatchAsyncCompute = false;
    PatchAntialiasing = false;
    PatchSkipStartMenu = false;
    PatchAmdSmt = false;
    PatchDisableIntroMovies = false;
    PatchDisableVignette = false;
    PatchDisableBoundaryTeleport = false;
    PatchDisableWin7Vsync = false;
    PatchMinimapFlicker = false;

    RemoveDeadBindings = true;
    EnableImGuiAssertions = false;
    PatchEnableDebug = false;
    DumpGameOptions = false;

    FontPath = "";
    FontGlyphRanges = "Default";
    FontSize = 18.0f;

    Save();
}

Options::Options(Paths& aPaths)
    : m_paths(aPaths)
{
    const auto* exePathStr = aPaths.Executable().native().c_str();
    const auto verInfoSz = GetFileVersionInfoSize(exePathStr, nullptr);
    if(verInfoSz)
    {
        const auto verInfo = std::make_unique<BYTE[]>(verInfoSz);
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

    set_default_logger(CreateLogger(GetAbsolutePath(L"cyber_engine_tweaks.log", m_paths.CETRoot(), true), "main", nullptr, "[%Y-%m-%d %H:%M:%S UTC%z] [%l] [%!] [%t] %v"));

    Log::Info("Cyber Engine Tweaks is starting...");

    GameImage.Initialize();

    if (GameImage.version)
    {
        Log::Info("CET version {} [{}]", CET_BUILD_COMMIT, CET_BUILD_BRANCH);
        auto [major, minor] = GameImage.GetVersion();
        Log::Info("Game version {}.{:02d}", major, minor);
        Log::Info("Root path: \"{}\"", UTF16ToUTF8(aPaths.GameRoot().native()));
        Log::Info("Cyber Engine Tweaks path: \"{}\"", UTF16ToUTF8(aPaths.CETRoot().native()));
        Log::Info("Lua scripts search path: \"{}\"", UTF16ToUTF8(aPaths.ModsRoot().native()));

        if (GameImage.GetVersion() != Image::GetSupportedVersion())
        {
            auto [smajor, sminor] = Image::GetSupportedVersion();
            Log::Error("Unsupported game version! Only {}.{:02d} is supported.", smajor, sminor);
            throw std::runtime_error("Unsupported version");
        }

    }
    else
    {
        Log::Info("Unknown Game Version, update the mod");
        throw std::runtime_error("Unknown version");
    }

    Load();
    Save();
}
