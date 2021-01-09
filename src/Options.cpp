#include <stdafx.h>

#include <spdlog/sinks/rotating_file_sink.h>

static std::unique_ptr<Options> s_pOptions;

Options::Options(HMODULE aModule)
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

    if (!IsCyberpunk2077())
        return;

    Logger::InfoToMain("Cyber Engine Tweaks is starting...");

    GameImage.Initialize();

    if (GameImage.version)
    {
        auto [major, minor] = GameImage.GetVersion();
        Logger::InfoToMainFmt("Game version {}.{:02d}", major, minor);
        Logger::InfoToMainFmt("Root path: \"{}\"", Paths::RootPath.string());
        Logger::InfoToMainFmt("Cyber Engine Tweaks path: \"{}\"", Paths::CETPath.string());
        Logger::InfoToMainFmt("Lua scripts search path: \"{}\"", Paths::ScriptsPath.string());
    }
    else
        Logger::InfoToMain("Unknown Game Version, update the mod");

    const auto configPath = Paths::CETPath / "config.json";
    
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
        this->ToolbarKey = config.value("console_key", this->ToolbarKey);

        // check old config names
        if (config.value("unlock_menu", false))
            this->PatchEnableDebug = true;

        this->ToolbarChar = MapVirtualKey(this->ToolbarKey, MAPVK_VK_TO_CHAR);
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
    config["console_key"] = this->ToolbarKey;
    config["disable_intro_movies"] = this->PatchDisableIntroMovies;
    config["disable_vignette"] = this->PatchDisableVignette;
    config["disable_boundary_teleport"] = this->PatchDisableBoundaryTeleport;
    config["disable_win7_vsync"] = this->PatchDisableWin7Vsync;

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
    s_pOptions.reset(new (std::nothrow) Options(aModule));
}

Options& Options::Get()
{
    assert(s_pOptions);
    return *s_pOptions;
}

