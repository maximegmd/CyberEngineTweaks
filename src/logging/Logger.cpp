#include <stdafx.h>

#include <spdlog/sinks/rotating_file_sink.h>

#include "overlay/Overlay.h"

using namespace std::chrono_literals;

static std::shared_ptr<spdlog::logger> s_mainLogger{ };
static std::shared_ptr<spdlog::logger> s_consoleLogger{ };

void Logger::Initialize()
{
    const auto mainRotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Paths::Get().CETRoot() / "cyber_engine_tweaks.log").string(), 1048576 * 5, 3);
    s_mainLogger = std::make_shared<spdlog::logger>("main", spdlog::sinks_init_list{ mainRotSink });
    s_mainLogger->set_pattern("[%H:%M:%S %z][%l] %v");
#ifdef CET_DEBUG
    s_mainLogger->flush_on(spdlog::level::trace);
#else
    s_mainLogger->flush_on(spdlog::level::err);
#endif

    const auto consoleRotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Paths::Get().CETRoot() / "console_dump.log").string(), 1048576 * 5, 3);
    s_consoleLogger = std::make_shared<spdlog::logger>("console", spdlog::sinks_init_list{ consoleRotSink });
    s_consoleLogger->set_pattern("[%H:%M:%S %z] %v");
    s_consoleLogger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(s_mainLogger);
    spdlog::flush_every(3s);

    Initialized = true;
}

void Logger::Shutdown()
{
    if (Initialized)
    {
        s_mainLogger->flush();
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

void Logger::TraceToMain(std::string_view aMsg)
{
    s_mainLogger->log(spdlog::level::trace, aMsg);
}

void Logger::DebugToMain(std::string_view aMsg)
{
    s_mainLogger->log(spdlog::level::debug, aMsg);
}

void Logger::InfoToMain(std::string_view aMsg)
{
    s_mainLogger->log(spdlog::level::info, aMsg);
}

void Logger::WarningToMain(std::string_view aMsg)
{
    s_mainLogger->log(spdlog::level::warn, aMsg);
}

void Logger::ErrorToMain(std::string_view aMsg)
{
    s_mainLogger->log(spdlog::level::err, aMsg);
}

void Logger::CriticalToMain(std::string_view aMsg)
{
    s_mainLogger->log(spdlog::level::critical, aMsg);
}
