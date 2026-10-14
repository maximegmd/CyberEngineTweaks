#pragma once

#if defined(__APPLE__) && !defined(_CRT_WIDE)
// CET macOS: the MSVC macro that makes a wide string literal.
#define _CRT_WIDE_(s) L##s
#define _CRT_WIDE(s) _CRT_WIDE_(s)
#endif

// global declaration "Enable ImGui Assertions"
extern bool g_ImGuiAssertionsEnabled;

#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD

// runtime assertions which can be enabled/disabled inside CET options
void ImGuiAssert(wchar_t const* acpMessage, wchar_t const* acpFile, unsigned aLine);

// custom assertion function macro for ImGui
#define IM_ASSERT(expression) (void)((g_ImGuiAssertionsEnabled && ((!!(expression)) || (ImGuiAssert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0))))