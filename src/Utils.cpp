#include <stdafx.h>

#include "Utils.h"

#include <spdlog/sinks/base_sink.h>
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

template <typename Mutex>
class CustomSink : public spdlog::sinks::base_sink<Mutex>
{
public:
    CustomSink(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler)
        : spdlog::sinks::base_sink<Mutex>()
        , m_sinkItHandler(aSinkItHandler)
        , m_flushHandler(aFlushHandler)
    {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        if (!m_sinkItHandler)
            return;
            
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        m_sinkItHandler(fmt::to_string(formatted));
    }

    void flush_() override
    {
        if (m_flushHandler)
            m_flushHandler();
    }

private:

    std::function<void(const std::string&)> m_sinkItHandler{ nullptr };
    std::function<void()> m_flushHandler{ nullptr };
};

template <typename Mutex>
spdlog::sink_ptr CreateCustomSink(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler)
{
    return std::make_shared<CustomSink<Mutex>>(aSinkItHandler, aFlushHandler);
}

spdlog::sink_ptr CreateCustomSinkST(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler)
{
    return CreateCustomSink<spdlog::details::null_mutex>(aSinkItHandler, aFlushHandler);
}

spdlog::sink_ptr CreateCustomSinkMT(std::function<void(const std::string&)> aSinkItHandler, std::function<void()> aFlushHandler)
{
    return CreateCustomSink<std::mutex>(aSinkItHandler, aFlushHandler);
}

std::shared_ptr<spdlog::logger> CreateLogger(const std::filesystem::path& aPath, const std::string& aID, spdlog::sink_ptr aExtraSink, const std::string& aPattern)
{
    auto existingLogger = spdlog::get(aID);
    if (existingLogger)
        return existingLogger;

    const auto rotSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(aPath.string(), 1048576 * 5, 3);
    rotSink->set_pattern(aPattern);
    auto logger = std::make_shared<spdlog::logger>(aID, spdlog::sinks_init_list{ rotSink });

    if (aExtraSink)
        logger->sinks().emplace_back(aExtraSink);

#ifdef CET_DEBUG
    logger->flush_on(spdlog::level::trace);
#else
    logger->flush_on(spdlog::level::err);
#endif

    register_logger(logger);
    return logger;
}
