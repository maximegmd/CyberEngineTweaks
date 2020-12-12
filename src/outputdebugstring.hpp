#pragma once
void outputDebugString(const wchar_t* fmt, ...);
void outputDebugString(const char* fmt, ...);

#if defined(_DEBUG) || defined(USE_DEBUG_TRACE)
#  define DEBUG_TRACE(...) outputDebugString(__VA_ARGS__)
#else
#  define DEBUG_TRACE(...)
#endif
