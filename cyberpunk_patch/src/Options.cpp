#include "Options.h"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <fstream>

Options::Options(HMODULE aModule)
{
    char path[2048 + 1] = { 0 };
    GetModuleFileNameA(aModule, path, std::size(path) - 1);

    Path = path;
    Path = Path.parent_path().parent_path();
    Path /= "performance_overhaul/";

    std::error_code ec;
    create_directories(Path, ec);

    const auto rotatingLogger = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Path / "performance_overhaul.log").string(), 1048576 * 5, 3);

    const auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ rotatingLogger });
    logger->flush_on(spdlog::level::debug);
    set_default_logger(logger);

    const auto configPath = Path / "config.json";

    std::ifstream configFile(configPath);
    if(configFile)
    {
        auto config = nlohmann::json::parse(configFile);
        this->PatchAVX = config.value("avx", this->PatchAVX);
        this->PatchSMT = config.value("smt", this->PatchSMT);
        this->PatchSpectre = config.value("spectre", this->PatchSpectre);
        this->PatchMemoryPool = config.value("memory_pool", this->PatchMemoryPool);
        this->PatchVirtualInput = config.value("virtual_input", this->PatchVirtualInput);
    }

    nlohmann::json config;
    config["avx"] = this->PatchAVX;
    config["smt"] = this->PatchSMT;
    config["spectre"] = this->PatchSpectre;
    config["memory_pool"] = this->PatchMemoryPool;
    config["virtual_input"] = this->PatchVirtualInput;

    std::ofstream o(configPath);
    o << config.dump(4) << std::endl;
}
