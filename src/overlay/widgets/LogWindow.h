#pragma once

struct D3D12;
struct LogWindow
{
    LogWindow(D3D12& aD3D12, const std::string& acpLoggerName);

    void Draw(const ImVec2& size);

private:
    D3D12& m_d3d12;
    std::string m_loggerName;

    void Log(const std::string& acpText);

    std::recursive_mutex m_lock;
    TiltedPhoques::Vector<std::pair<spdlog::level::level_enum, std::string>> m_lines;
    float m_normalizedWidth{ -1.0f };
    bool m_shouldScroll{ true };
    bool m_scroll{ false };
};
