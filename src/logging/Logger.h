#pragma once

#include <spdlog/fmt/fmt.h>

struct Logger
{
    Logger() = delete;
    ~Logger() = delete;

    static void Initialize();
    static void Shutdown();

    static void GameToConsole(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void GameToConsoleFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        GameToConsole(fmt::format(aFormat, aArgs...));
    }
          
    static void ToConsole(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void ToConsoleFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        ToConsole(fmt::format(aFormat, aArgs...));
    }
        
    static inline bool Initialized{ false };
};