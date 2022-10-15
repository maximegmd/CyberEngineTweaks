#pragma once

// global declaration "Enable ImGui Assertions"
extern bool g_ImGuiAssertionsEnabled;

// runtime assertions which can be enabled/disabled inside CET options
void ImGuiAssert(wchar_t const* acpMessage, wchar_t const* acpFile, unsigned aLine);

// custom assertion function macro for ImGui
#define IM_ASSERT(expression) (void)(                                                    \
    (((!!(expression)) ||                                                                \
    (ImGuiAssert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0))))