#pragma once

#include "Widget.h"

struct LogWindow : Widget
{
    LogWindow(const Options& acOptions, const std::string& acpWindowTitle, const std::string& acpLoggerName);

protected:
    void OnUpdate() override;

    void DrawLog(const ImVec2& size);

    const Options& m_options;

private:
    void Log(const std::string& acpText);

    std::string m_loggerName;
    float m_normalizedWidth{ -1.0f };
    bool m_shouldScroll{ true };

    std::recursive_mutex m_lock;
    TiltedPhoques::Vector<std::pair<spdlog::level::level_enum, std::string>> m_lines;
    size_t m_nextIndexToCheck{ 0 };
    bool m_scroll{ false };
};
