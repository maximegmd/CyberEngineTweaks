#include <stdafx.h>

#include <spdlog/sinks/rotating_file_sink.h>

#include "toolbar/Toolbar.h"

using namespace std::chrono_literals;

static std::shared_ptr<spdlog::logger> s_mainLogger{ };
static std::shared_ptr<spdlog::logger> s_consoleLogger{ };
static std::shared_ptr<spdlog::logger> s_modsLogger{ };

void Logger::Initialize()
{
    if (!Paths::Initialized)
        return;

    const auto mainRotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Paths::CETPath / "cyber_engine_tweaks.log").string(), 1048576 * 5, 3);
    s_mainLogger = std::make_shared<spdlog::logger>("main", spdlog::sinks_init_list{ mainRotSink });
    s_mainLogger->set_pattern("[%H:%M:%S %z][%l] %v");
#ifdef CET_DEBUG
    s_mainLogger->flush_on(spdlog::level::trace);
#else
    s_mainLogger->flush_on(spdlog::level::err);
#endif

    const auto consoleRotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Paths::CETPath / "console_dump.log").string(), 1048576 * 5, 3);
    s_consoleLogger = std::make_shared<spdlog::logger>("console", spdlog::sinks_init_list{ consoleRotSink });
    s_consoleLogger->set_pattern("[%H:%M:%S %z] %v");
    s_consoleLogger->flush_on(spdlog::level::info);

    const auto modsRotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Paths::CETPath / "mods_dump.log").string(), 1048576 * 5, 3);
    s_modsLogger = std::make_shared<spdlog::logger>("mods", spdlog::sinks_init_list{ modsRotSink });
    s_modsLogger->set_pattern("[%H:%M:%S %z][%l] %v");
#ifdef CET_DEBUG
    s_modsLogger->flush_on(spdlog::level::trace);
#else
    s_modsLogger->flush_on(spdlog::level::err);
#endif

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
        s_modsLogger->flush();
    }
}
    
void Logger::GameToConsole(std::string_view aMsg)
{
    //Toolbar::GetConsole().GameLog(aMsg);
    Toolbar::GetConsole().GameLog(aMsg);
}
    
void Logger::ToConsole(std::string_view aMsg)
{
    s_consoleLogger->log(spdlog::level::info, aMsg);
    Toolbar::GetConsole().Log(aMsg);
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

void Logger::TraceToMods(std::string_view aMsg)
{
    s_modsLogger->log(spdlog::level::trace, aMsg);
}

void Logger::DebugToMods(std::string_view aMsg)
{
    s_modsLogger->log(spdlog::level::debug, aMsg);
}

void Logger::InfoToMods(std::string_view aMsg)
{
    s_modsLogger->log(spdlog::level::info, aMsg);
}

void Logger::WarningToMods(std::string_view aMsg)
{
    s_modsLogger->log(spdlog::level::warn, aMsg);
}

void Logger::ErrorToMods(std::string_view aMsg)
{
    s_modsLogger->log(spdlog::level::err, aMsg);
}

void Logger::CriticalToMods(std::string_view aMsg)
{
    s_modsLogger->log(spdlog::level::critical, aMsg);
}
