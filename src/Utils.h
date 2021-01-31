#pragma once

void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);

spdlog::sink_ptr CreateCustomSinkST(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler = nullptr);
spdlog::sink_ptr CreateCustomSinkMT(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler = nullptr);
std::shared_ptr<spdlog::logger> CreateLogger(const std::filesystem::path& aPath, const std::string& aID, spdlog::sink_ptr aExtraSink = nullptr, const std::string& aPattern = "[%Y-%m-%d %H:%M:%S UTC%z] [%l] %v");

template <typename ...Args>
void TryLuaFunction(std::shared_ptr<spdlog::logger> logger, sol::function func, Args... args)
{
    if (!func)
        return;
    
    // TODO: proper exception handling!
    sol::protected_function_result result;
    try
    {
        result = func(args...);
    }
    catch(std::exception& e)
    {
        logger->error(e.what());
    }
    if (!result.valid())
    {
        sol::error error = result;
        logger->error(error.what());
    }
}