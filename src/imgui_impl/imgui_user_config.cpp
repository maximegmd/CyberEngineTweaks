#include <stdafx.h>

#ifdef __APPLE__
#include "Utils.h"
#endif

// NOTE: imgui_user_config.h is included by imgui.h which is included with precompiled header, so no need to include it
// here once more

// global definition "Enable ImGui Assertions Logging"
bool g_ImGuiAssertionsEnabled{false};

#ifndef __APPLE__
#ifdef NDEBUG
// inline _wassert decl for NDEBUG as it is not emitted inside assert.h header in this case
extern "C" _ACRTIMP void __cdecl _wassert(wchar_t const* _Message, wchar_t const* _File, unsigned _Line);
#endif
#endif

// runtime assertions which can be enabled/disabled inside CET options, always logged into main log file when they
// happen
void ImGuiAssert(wchar_t const* acpMessage, wchar_t const* acpFile, unsigned aLine)
{
    // TODO - make this log to log of the one who caused assertion instead of default log!
#ifdef __APPLE__
    // CET macOS: spdlog has no wide-string support outside Windows.
    spdlog::error("ImGui assertion failed in file \"{}\" at line {}! Expression ({}) evaluates to false!", UTF16ToUTF8(acpFile), aLine,
                  UTF16ToUTF8(acpMessage));
#else
    spdlog::error(L"ImGui assertion failed in file \"{}\" at line {}! Expression ({}) evaluates to false!", acpFile, aLine, acpMessage);
#endif

#ifndef __APPLE__
#ifdef CET_DEBUG
    // we want to truly assert only in debug build, as it is causing confusion when users enable it...
    _wassert(acpMessage, acpFile, aLine);
#endif
#endif
}
