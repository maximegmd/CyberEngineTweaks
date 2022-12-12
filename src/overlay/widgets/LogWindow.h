#pragma once

struct LogWindow
{
    LogWindow(const std::string& acpLoggerName);

    void Draw(const ImVec2& size);

private:
    void Log(const std::string& acpText);

    std::string m_loggerName;
    float m_normalizedWidth{-1.0f};
    bool m_shouldScroll{true};

    std::recursive_mutex m_lock;
    TiltedPhoques::Vector<std::pair<spdlog::level::level_enum, std::string>> m_lines;
    size_t m_nextIndexToCheck{0};
    bool m_scroll{false};
};
