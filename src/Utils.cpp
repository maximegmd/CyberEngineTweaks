#include <stdafx.h>

#include "Utils.h"

#include <spdlog/sinks/rotating_file_sink.h>

void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
}

void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}


std::shared_ptr<spdlog::logger> CreateLogger(const std::filesystem::path& path, const std::string& id, const std::string& pattern)
{
    const auto rotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(path.string(), 1048576 * 5, 3);
    auto logger = std::make_shared<spdlog::logger>(id, spdlog::sinks_init_list{ rotSink });
    logger->set_pattern(pattern);
#ifdef CET_DEBUG
    logger->flush_on(spdlog::level::trace);
#else
    logger->flush_on(spdlog::level::err);
#endif
    return logger;
}
