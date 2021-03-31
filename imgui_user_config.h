#pragma once

void ImGuiAssert(wchar_t const* acpMessage, wchar_t const* acpFile, unsigned aLine);

#define IM_ASSERT(expression) (void)((!!(expression)) || (ImGuiAssert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0))