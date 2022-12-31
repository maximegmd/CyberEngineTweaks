#include "stdafx.h"

// NOTE: imgui_user_config.h is included by imgui.h which is included with precompiled header, so no need to include it
// here once more

// global definition "Enable ImGui Assertions Logging"
bool g_ImGuiAssertionsEnabled{false};

#ifdef NDEBUG
// inline _wassert decl for NDEBUG as it is not emitted inside assert.h header in this case
extern "C" _ACRTIMP void __cdecl _wassert(wchar_t const* _Message, wchar_t const* _File, unsigned _Line);
#endif

// runtime assertions which can be enabled/disabled inside CET options, always logged into main log file when they
// happen
void ImGuiAssert(wchar_t const* acpMessage, wchar_t const* acpFile, unsigned aLine)
{
    // TODO - make this log to log of the one who caused assertion instead of default log!
    spdlog::error(L"ImGui assertion failed in file \"{}\" at line {}! Expression ({}) evaluates to false!", acpFile, aLine, acpMessage);

#ifdef CET_DEBUG
    // we want to truly assert only in debug build, as it is causing confusion when users enable it...
    _wassert(acpMessage, acpFile, aLine);
#endif
}
