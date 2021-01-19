#include <stdafx.h>

#include <spdlog/sinks/rotating_file_sink.h>

#include "overlay/Overlay.h"

static std::shared_ptr<spdlog::logger> s_consoleLogger{ };

void Logger::Initialize()
{
    const auto consoleRotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Paths::Get().CETRoot() / "console_dump.log").string(), 1048576 * 5, 3);
    s_consoleLogger = std::make_shared<spdlog::logger>("console", spdlog::sinks_init_list{ consoleRotSink });
    s_consoleLogger->set_pattern("[%H:%M:%S %z] %v");
    s_consoleLogger->flush_on(spdlog::level::info);
    
    Initialized = true;
}

void Logger::Shutdown()
{
    if (Initialized)
    {
        s_consoleLogger->flush();
    }
}
    
void Logger::GameToConsole(std::string_view aMsg)
{
    //Overlay::GetConsole().GameLog(aMsg);
    Overlay::GetConsole().GameLog(aMsg);
}
    
void Logger::ToConsole(std::string_view aMsg)
{
    s_consoleLogger->log(spdlog::level::info, aMsg);
    Overlay::GetConsole().Log(aMsg);
}
