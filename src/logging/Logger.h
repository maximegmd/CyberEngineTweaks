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

    static void TraceToMain(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void TraceToMainFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        TraceToMain(fmt::format(aFormat, aArgs...));
    }
    static void DebugToMain(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void DebugToMainFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        DebugToMain(fmt::format(aFormat, aArgs...));
    }
    static void InfoToMain(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void InfoToMainFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        InfoToMain(fmt::format(aFormat, aArgs...));
    }
    static void WarningToMain(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void WarningToMainFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        WarningToMain(fmt::format(aFormat, aArgs...));
    }
    static void ErrorToMain(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void ErrorToMainFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        ErrorToMain(fmt::format(aFormat, aArgs...));
    }
    static void CriticalToMain(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void CriticalToMainFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        CriticalToMain(fmt::format(aFormat, aArgs...));
    }

    static void TraceToMods(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void TraceToModsFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        TraceToMods(fmt::format(aFormat, aArgs...));
    }
    static void DebugToMods(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void DebugToModsFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        DebugToMods(fmt::format(aFormat, aArgs...));
    }
    static void InfoToMods(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void InfoToModsFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        InfoToMods(fmt::format(aFormat, aArgs...));
    }
    static void WarningToMods(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void WarningToModsFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        WarningToMods(fmt::format(aFormat, aArgs...));
    }
    static void ErrorToMods(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void ErrorToModsFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        ErrorToMods(fmt::format(aFormat, aArgs...));
    }
    static void CriticalToMods(std::string_view aMsg);
    template<typename FormatString, typename... Args>
    static void CriticalToModsFmt(const FormatString &aFormat, Args&&... aArgs)
    {
        CriticalToMods(fmt::format(aFormat, aArgs...));
    }
    
    static inline bool Initialized{ false };
};