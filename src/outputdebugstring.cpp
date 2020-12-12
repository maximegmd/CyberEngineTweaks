#include "common.hpp"
#include <array>

#if defined(USE_OUTOUT_DEBUG_STRING) && (USE_OUTOUT_DEBUG_STRING == 1)
void outputDebugString(const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::array<wchar_t, 1024> buf;
    const auto n = swprintf_s(buf.data(), buf.size(), L"" APPNAME ": ");
    vswprintf_s(buf.data()+n, buf.size()-n, fmt, args);
    OutputDebugStringW(buf.data());
    va_end(args);
}

void outputDebugString(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::array<char, 1024> buf;
    const auto n = sprintf_s(buf.data(), buf.size(), "" APPNAME ": ");
    vsprintf_s(buf.data()+n, buf.size()-n, fmt, args);
    OutputDebugStringA(buf.data());
    va_end(args);
}

#else

void outputDebugString(const wchar_t*, ...) {}
void outputDebugString(const char*, ...) {}

#endif
