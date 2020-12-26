#include "Options.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fstream>

static std::unique_ptr<Options> s_instance;

Options::Options(HMODULE aModule)
{
    char path[2048 + 1] = { 0 };
    GetModuleFileNameA(aModule, path, std::size(path) - 1);

    Path = path;

    char parentPath[2048 + 1] = { 0 };
    GetModuleFileNameA(GetModuleHandleA(nullptr), parentPath, std::size(parentPath) - 1);

    ExeName = std::filesystem::path(parentPath).filename().string();
    if (!IsCyberpunk2077())
        return;

    Path = Path.parent_path().parent_path();

    Path /= "plugins";
    Path /= "cyber_engine_tweaks/";

    std::error_code ec;
    create_directories(Path, ec);

    const auto rotatingLogger = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Path / "cyber_engine_tweaks.log").string(), 1048576 * 5, 3);

    const auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ rotatingLogger });
    logger->flush_on(spdlog::level::debug);
    set_default_logger(logger);

    GameImage.Initialize();

    if (GameImage.version)
    {
        auto [major, minor] = GameImage.GetVersion();
        spdlog::info("Game version {}.{:02d}", major, minor);
    }
    else
        spdlog::info("Unknown Game Version, update the mod");

    const auto configPath = Path / "config.json";

    std::ifstream configFile(configPath);
    if(configFile)
    {
        auto config = nlohmann::json::parse(configFile);
        this->PatchAVX = config.value("avx", this->PatchAVX);
        this->PatchSMT = config.value("smt", this->PatchSMT);
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
}

bool Options::IsCyberpunk2077() const noexcept
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

