#include "Options.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
//#include <spdlog/sinks/rotating_file_sink.h>
#include <fstream>

static std::unique_ptr<Options> s_instance;

//char Options::le_DLL_Path[MAX_PATH];
LPCSTR Options::TargetAppWindowTitle = "Cyberpunk 2077 (C) 2020 by CD Projekt RED";
std::filesystem::path Options::Path;
std::string Options::ExeName;

bool Options::IsCyberpunk2077()/* const noexcept*/
{
    return ExeName == "Cyberpunk2077.exe";
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

Options::Options(HMODULE aModule)
{
    GameImage.Initialize();

    if (GameImage.version)
    {
        auto [major, minor] = GameImage.GetVersion();
        spdlog::info("Game version {}.{:02d}", major, minor);
    }
    else
        spdlog::info("Unknown Game Version, update the mod");

    const auto configPath = Path / "config.json";
    spdlog::info("configPath = {0}{1}", Path.string().c_str(), "config.json");

    std::ifstream configFile(configPath);
    if(configFile)
    {
        auto config = nlohmann::json::parse(configFile);
        this->PatchAVX = config.value("avx", this->PatchAVX);
        this->PatchSMT = config.value("smt", this->PatchSMT);
        this->PatchMemoryPool = config.value("memory_pool", this->PatchMemoryPool);
        this->PatchVirtualInput = config.value("virtual_input", this->PatchVirtualInput);
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

        this->DumpGameOptions = config.value("dump_game_options", this->DumpGameOptions);
        this->Console = config.value("console", this->Console);
		this->ConsoleKey = config.value("console_key", this->ConsoleKey);

        // check old config names
        if (config.value("unlock_menu", false))
            this->PatchEnableDebug = true;

        this->ConsoleChar = MapVirtualKeyA(this->ConsoleKey, MAPVK_VK_TO_CHAR);
    }

    nlohmann::json config;
    config["avx"] = this->PatchAVX;
    config["smt"] = this->PatchSMT;
    config["memory_pool"] = this->PatchMemoryPool;
    config["virtual_input"] = this->PatchVirtualInput;
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

    std::ofstream o(configPath);
    o << config.dump(4) << std::endl;

    spdlog::info("config loaded OK");
}
