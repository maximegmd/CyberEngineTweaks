#include <stdafx.h>

#include "Options.h"

#include <spdlog/sinks/rotating_file_sink.h>

static std::unique_ptr<Options> s_instance;

Options::Options(HMODULE aModule)
{
    TCHAR exePathBuf[2048 + 1] = { 0 };
    GetModuleFileName(GetModuleHandle(nullptr), exePathBuf, std::size(exePathBuf) - 1);

    int verInfoSz = GetFileVersionInfoSize(exePathBuf, nullptr);
    if(verInfoSz) 
    {
        auto verInfo = std::make_unique<BYTE[]>(verInfoSz);
        if(GetFileVersionInfo(exePathBuf, 0, verInfoSz, verInfo.get())) 
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
    std::filesystem::path exePath = exePathBuf;
    ExeValid = ExeValid || (exePath.filename() == "Cyberpunk2077.exe");

    if (!IsCyberpunk2077())
        return;

    RootPath = exePath.parent_path();

    CETPath = RootPath;
    CETPath /= "plugins";
    CETPath /= "cyber_engine_tweaks";
    std::filesystem::create_directories(CETPath);

    ScriptsPath = CETPath / "mods";
    std::filesystem::create_directories(ScriptsPath);

    const auto rotatingLogger = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((CETPath / "cyber_engine_tweaks.log").string(), 1048576 * 5, 3);

    const auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ rotatingLogger });
    logger->flush_on(spdlog::level::debug);
    set_default_logger(logger);

    spdlog::info("Cyber Engine Tweaks is starting...");

    GameImage.Initialize();

    if (GameImage.version)
    {
        auto [major, minor] = GameImage.GetVersion();
        spdlog::info("Game version {}.{:02d}", major, minor);
        spdlog::info("Root path: \"{}\"", RootPath.string().c_str());
        spdlog::info("Cyber Engine Tweaks path: \"{}\"", CETPath.string().c_str());
        spdlog::info("Lua scripts search path: \"{}\"", ScriptsPath.string().c_str());
    }
    else
        spdlog::info("Unknown Game Version, update the mod");

    const auto configPath = CETPath / "config.json";
    
    // remove empty config.json
    if (std::filesystem::exists(configPath) && !std::filesystem::file_size(configPath))
        std::filesystem::remove(configPath);

    std::ifstream configFile(configPath);
    if(configFile)
    {
        auto config = nlohmann::json::parse(configFile);
        this->PatchEnableDebug = config.value("enable_debug", this->PatchEnableDebug);
        this->CPUMemoryPoolFraction = config.value("cpu_memory_pool_fraction", this->CPUMemoryPoolFraction);
        this->GPUMemoryPoolFraction = config.value("gpu_memory_pool_fraction", this->GPUMemoryPoolFraction);
        this->PatchRemovePedestrians = config.value("remove_pedestrians", this->PatchRemovePedestrians);
        this->PatchSkipStartMenu = config.value("skip_start_menu", this->PatchSkipStartMenu);
        this->PatchAsyncCompute = config.value("disable_async_compute", this->PatchAsyncCompute);
        this->PatchAntialiasing = config.value("disable_antialiasing", this->PatchAntialiasing);
        this->PatchDisableIntroMovies = config.value("disable_intro_movies", this->PatchDisableIntroMovies);
        this->PatchDisableVignette = config.value("disable_vignette", this->PatchDisableVignette);
        this->PatchDisableBoundaryTeleport = config.value("disable_boundary_teleport", this->PatchDisableBoundaryTeleport);
        this->PatchDisableWin7Vsync = config.value("disable_win7_vsync", this->PatchDisableWin7Vsync);

        this->DumpGameOptions = config.value("dump_game_options", this->DumpGameOptions);
        this->Console = config.value("console", this->Console);
        this->ConsoleKey = config.value("console_key", this->ConsoleKey);

        // font config
        this->FontPath = config.value("font_path", this->FontPath);
        this->FontGlyphRanges = config.value("font_glyph_ranges", this->FontGlyphRanges);
        this->FontSize = config.value("font_size", this->FontSize);

        // check old config names
        if (config.value("unlock_menu", false))
            this->PatchEnableDebug = true;

        this->ConsoleChar = MapVirtualKey(this->ConsoleKey, MAPVK_VK_TO_CHAR);
    }
    configFile.close();

    nlohmann::json config;
    config["enable_debug"] = this->PatchEnableDebug;
    config["cpu_memory_pool_fraction"] = this->CPUMemoryPoolFraction;
    config["gpu_memory_pool_fraction"] = this->GPUMemoryPoolFraction;
    config["remove_pedestrians"] = this->PatchRemovePedestrians;
    config["skip_start_menu"] = this->PatchSkipStartMenu;
    config["disable_async_compute"] = this->PatchAsyncCompute;
    config["disable_antialiasing"] = this->PatchAntialiasing;
    config["dump_game_options"] = this->DumpGameOptions;
    config["console"] = this->Console;
    config["console_key"] = this->ConsoleKey;
    config["disable_intro_movies"] = this->PatchDisableIntroMovies;
    config["disable_vignette"] = this->PatchDisableVignette;
    config["disable_boundary_teleport"] = this->PatchDisableBoundaryTeleport;
    config["disable_win7_vsync"] = this->PatchDisableWin7Vsync;
    config["font_path"] = this->FontPath;
    config["font_glyph_ranges"] = this->FontGlyphRanges;
    config["font_size"] = this->FontSize;

    std::ofstream o(configPath);
    o << config.dump(4) << std::endl;
}

bool Options::IsCyberpunk2077() const noexcept
{
    return ExeValid;
}

void Options::Initialize(HMODULE aModule)
{
    // Horrible hack because make_unique can't access private member
    s_instance.reset(new (std::nothrow) Options(aModule));
}

Options& Options::Get()
{
    return *s_instance;
}

