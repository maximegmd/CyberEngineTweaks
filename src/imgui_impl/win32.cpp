// dear imgui: Platform Backend for Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support (for Win32 this is actually part of core dear imgui)
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using VK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(VK_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

// You can copy and use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include <stdafx.h>

#include "win32.h"

#include "CET.h"

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2020-12-04: Misc: Fixed setting of io.DisplaySize to invalid/uninitialized data when after hwnd has been closed.
//  2020-03-03: Inputs: Calling AddInputCharacterUTF16() to support surrogate pairs leading to codepoint >= 0x10000 (for more complete CJK inputs)
//  2020-02-17: Added ImGui_ImplWin32_EnableDpiAwareness(), ImGui_ImplWin32_GetDpiScaleForHwnd(), ImGui_ImplWin32_GetDpiScaleForMonitor() helper functions.
//  2019-12-05: Inputs: Added support for ImGuiMouseCursor_NotAllowed mouse cursor.
//  2019-05-11: Inputs: Don't filter value from WM_CHAR before calling AddInputCharacter().
//  2019-01-17: Misc: Using GetForegroundWindow()+IsChild() instead of GetActiveWindow() to be compatible with windows created in a different thread or parent.
//  2019-01-17: Inputs: Added support for mouse buttons 4 and 5 via WM_XBUTTON* messages.
//  2019-01-15: Inputs: Added support for XInput gamepads (if ImGuiConfigFlags_NavEnableGamepad is set by user application).
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-06-29: Inputs: Added support for the ImGuiMouseCursor_Hand cursor.
//  2018-06-10: Inputs: Fixed handling of mouse wheel messages to support fine position messages (typically sent by track-pads).
//  2018-06-08: Misc: Extracted imgui_impl_win32.cpp/.h away from the old combined DX9/DX10/DX11/DX12 examples.
//  2018-03-20: Misc: Setup io.BackendFlags ImGuiBackendFlags_HasMouseCursors and ImGuiBackendFlags_HasSetMousePos flags + honor ImGuiConfigFlags_NoMouseCursorChange flag.
//  2018-02-20: Inputs: Added support for mouse cursors (ImGui::GetMouseCursor() value and WM_SETCURSOR message handling).
//  2018-02-06: Inputs: Added mapping for ImGuiKey_Space.
//  2018-02-06: Inputs: Honoring the io.WantSetMousePos by repositioning the mouse (when using navigation and ImGuiConfigFlags_NavMoveMouse is set).
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2018-01-20: Inputs: Added Horizontal Mouse Wheel support.
//  2018-01-08: Inputs: Added mapping for ImGuiKey_Insert.
//  2018-01-05: Inputs: Added WM_LBUTTONDBLCLK double-click handlers for window classes with the CS_DBLCLKS flag.
//  2017-10-23: Inputs: Added WM_SYSKEYDOWN / WM_SYSKEYUP handlers so e.g. the VK_MENU key can be read.
//  2017-10-23: Inputs: Using Win32 ::SetCapture/::GetCapture() to retrieve mouse positions outside the client area when dragging.
//  2016-11-12: Inputs: Only call Win32 ::SetCursor(NULL) when io.MouseDrawCursor is set.

// Win32 Data
static HWND             g_hWnd = NULL;
static INT64            g_Time = 0;
static INT64            g_TicksPerSecond = 0;
static ImGuiMouseCursor g_LastMouseCursor = ImGuiMouseCursor_COUNT;
static std::string      g_LayoutPath = "";

// Functions
bool ImGui_ImplWin32_Init(HWND ahWnd)
{
    if (!::QueryPerformanceFrequency((LARGE_INTEGER*)&g_TicksPerSecond))
        return false;
    if (!::QueryPerformanceCounter((LARGE_INTEGER*)&g_Time))
        return false;

    // Setup backend capabilities flags
    g_hWnd = ahWnd;
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendPlatformName = "imgui_impl_win32";
    io.ImeWindowHandle = ahWnd;

    // Setup ini path
    g_LayoutPath = (CET::Get().GetPaths().CETRoot() / "layout.ini").string();
    io.IniFilename = g_LayoutPath.c_str();

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    return true;
}

void ImGui_ImplWin32_Shutdown()
{
    g_hWnd = (HWND)0;
}

static bool ImGui_ImplWin32_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return false;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        ::SetCursor(NULL);
    }
    else
    {
        // Show OS mouse cursor
        LPTSTR win32_cursor = IDC_ARROW;
        switch (imgui_cursor)
        {
        case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
        case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
        case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
        case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
        case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
        case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
        case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
        case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
        case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
        }
        ::SetCursor(::LoadCursor(NULL, win32_cursor));
    }
    return true;
}

static void ImGui_ImplWin32_UpdateMousePos(SIZE aOutSize)
{
    ImGuiIO& io = ImGui::GetIO();

    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos)
    {
        POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
        if (::ClientToScreen(g_hWnd, &pos))
            ::SetCursorPos(pos.x, pos.y);
    }

    // Set mouse position
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    POINT pos;
    if (HWND active_window = ::GetForegroundWindow())
        if (active_window == g_hWnd || ::IsChild(active_window, g_hWnd))
            if (::GetCursorPos(&pos) && ::ScreenToClient(g_hWnd, &pos))
                if (!aOutSize.cx || !aOutSize.cy)
                    io.MousePos = ImVec2((float)pos.x, (float)pos.y);
                else
                {
                    RECT clientRect;
                    ::GetClientRect(g_hWnd, &clientRect);

                    // scale, just to make sure coords are correct (fixes issues in fullscreen)
                    auto xScale = static_cast<float>(aOutSize.cx) / (clientRect.right - clientRect.left);
                    auto yScale = static_cast<float>(aOutSize.cy) / (clientRect.bottom - clientRect.top);
                    io.MousePos = ImVec2(pos.x * xScale, pos.y * yScale);
                }
}

void ImGui_ImplWin32_NewFrame(SIZE aOutSize)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = { static_cast<float>(aOutSize.cx), static_cast<float>(aOutSize.cy) };

    // Setup time step
    INT64 current_time = 0;
    ::QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
    io.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (::GetKeyState(VK_MENU) & 0x8000) != 0;
    io.KeySuper = false;
    // io.KeysDown[], io.MousePos, io.MouseDown[], io.MouseWheel: filled by the WndProc handler below.

    // Update OS mouse position
    ImGui_ImplWin32_UpdateMousePos(aOutSize);

    // Update OS mouse cursor with the cursor requested by imgui
    ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
    if (g_LastMouseCursor != mouse_cursor)
    {
        g_LastMouseCursor = mouse_cursor;
        ImGui_ImplWin32_UpdateMouseCursor();
    }
}

// Win32 message handler (process Win32 mouse/keyboard inputs, etc.)
// Call from your application's message handler.
// When implementing your own backend, you can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if Dear ImGui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to Dear ImGui, and hide them from your application based on those two flags.
// PS: In this Win32 handler, we use the capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinates when dragging mouse outside of our window bounds.
// PS: We treat DBLCLK messages as regular mouse down messages, so this code will work on windows classes that have the CS_DBLCLKS flag set. Our own example app code doesn't set this flag.
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam)
{
    if (ImGui::GetCurrentContext() == NULL)
        return 0;

    ImGuiIO& io = ImGui::GetIO();
    switch (auMsg)
    {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
        int button = 0;
        if (auMsg == WM_LBUTTONDOWN || auMsg == WM_LBUTTONDBLCLK) { button = 0; }
        if (auMsg == WM_RBUTTONDOWN || auMsg == WM_RBUTTONDBLCLK) { button = 1; }
        if (auMsg == WM_MBUTTONDOWN || auMsg == WM_MBUTTONDBLCLK) { button = 2; }
        if (auMsg == WM_XBUTTONDOWN || auMsg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(awParam) == XBUTTON1) ? 3 : 4; }
        if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
            ::SetCapture(ahWnd);
        io.MouseDown[button] = true;
        return 0;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        int button = 0;
        if (auMsg == WM_LBUTTONUP) { button = 0; }
        if (auMsg == WM_RBUTTONUP) { button = 1; }
        if (auMsg == WM_MBUTTONUP) { button = 2; }
        if (auMsg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(awParam) == XBUTTON1) ? 3 : 4; }
        io.MouseDown[button] = false;
        if (!ImGui::IsAnyMouseDown() && ::GetCapture() == ahWnd)
            ::ReleaseCapture();
        return 0;
    }
    case WM_MOUSEWHEEL:
        io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(awParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_MOUSEHWHEEL:
        io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(awParam) / (float)WHEEL_DELTA;
        return 0;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (awParam < 256)
            io.KeysDown[awParam] = 1;
        return 0;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (awParam < 256)
            io.KeysDown[awParam] = 0;
        return 0;
    case WM_CHAR:
        // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
        if (awParam > 0 && awParam < 0x10000)
            io.AddInputCharacterUTF16((unsigned short)awParam);
        return 0;
    case WM_SETCURSOR:
        if (LOWORD(alParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
            return 1;
        return 0;
    }
    return 0;
}


//--------------------------------------------------------------------------------------------------------
// DPI-related helpers (optional)
//--------------------------------------------------------------------------------------------------------
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
//---------------------------------------------------------------------------------------------------------
// This is the scheme successfully used by GLFW (from which we borrowed some of the code) and other apps aiming to be highly portable.
// ImGui_ImplWin32_EnableDpiAwareness() is just a helper called by main.cpp, we don't call it automatically.
// If you are trying to implement your own backend for your own engine, you may ignore that noise.
//---------------------------------------------------------------------------------------------------------

// Implement some of the functions and types normally declared in recent Windows SDK.
#if !defined(_versionhelpers_H_INCLUDED_) && !defined(_INC_VERSIONHELPERS)
static BOOL IsWindowsVersionOrGreater(WORD aMajor, WORD aMinor, WORD aSP)
{
    OSVERSIONINFOEXW osvi = { sizeof(osvi), aMajor, aMinor, 0, 0, { 0 }, aSP, 0, 0, 0, 0 };
    DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR;
    ULONGLONG cond = ::VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
    cond = ::VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
    cond = ::VerSetConditionMask(cond, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    return ::VerifyVersionInfoW(&osvi, mask, cond);
}
#define IsWindows8Point1OrGreater()  IsWindowsVersionOrGreater(HIBYTE(0x0602), LOBYTE(0x0602), 0) // _WIN32_WINNT_WINBLUE
#endif

#ifndef DPI_ENUMS_DECLARED
typedef enum { PROCESS_DPI_UNAWARE = 0, PROCESS_SYSTEM_DPI_AWARE = 1, PROCESS_PER_MONITOR_DPI_AWARE = 2 } PROCESS_DPI_AWARENESS;
typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
#endif
#ifndef _DPI_AWARENESS_CONTEXTS_
DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    (DPI_AWARENESS_CONTEXT)-3
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 (DPI_AWARENESS_CONTEXT)-4
#endif
typedef HRESULT(WINAPI* PFN_SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS);                     // Shcore.lib + dll, Windows 8.1+
typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);        // Shcore.lib + dll, Windows 8.1+
typedef DPI_AWARENESS_CONTEXT(WINAPI* PFN_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT); // User32.lib + dll, Windows 10 v1607+ (Creators Update)

// Helper function to enable DPI awareness without setting up a manifest
void ImGui_ImplWin32_EnableDpiAwareness()
{
    // if (IsWindows10OrGreater()) // This needs a manifest to succeed. Instead we try to grab the function pointer!
    {
        static HINSTANCE user32_dll = ::LoadLibrary(_T("user32.dll")); // Reference counted per-process
        if (PFN_SetThreadDpiAwarenessContext SetThreadDpiAwarenessContextFn = (PFN_SetThreadDpiAwarenessContext)::GetProcAddress(user32_dll, "SetThreadDpiAwarenessContext"))
        {
            SetThreadDpiAwarenessContextFn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            return;
        }
    }
    if (IsWindows8Point1OrGreater())
    {
        static HINSTANCE shcore_dll = ::LoadLibrary(_T("shcore.dll")); // Reference counted per-process
        if (PFN_SetProcessDpiAwareness SetProcessDpiAwarenessFn = (PFN_SetProcessDpiAwareness)::GetProcAddress(shcore_dll, "SetProcessDpiAwareness"))
        {
            SetProcessDpiAwarenessFn(PROCESS_PER_MONITOR_DPI_AWARE);
            return;
        }
    }
#if _WIN32_WINNT >= 0x0600
    ::SetProcessDPIAware();
#endif
}

#if defined(_MSC_VER) && !defined(NOGDI)
#pragma comment(lib, "gdi32")   // Link with gdi32.lib for GetDeviceCaps()
#endif

float ImGui_ImplWin32_GetDpiScaleForMonitor(HMONITOR ahMonitor)
{
    UINT xdpi = 96, ydpi = 96;
    static BOOL bIsWindows8Point1OrGreater = IsWindows8Point1OrGreater();
    if (bIsWindows8Point1OrGreater)
    {
        static HINSTANCE shcore_dll = ::LoadLibrary(_T("shcore.dll")); // Reference counted per-process
        if (PFN_GetDpiForMonitor GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor"))
            GetDpiForMonitorFn(ahMonitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
    }
#ifndef NOGDI
    else
    {
        const HDC dc = ::GetDC(NULL);
        xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
        ::ReleaseDC(NULL, dc);
    }
#endif
    IM_ASSERT(xdpi == ydpi); // Please contact me if you hit this assert!
    return xdpi / 96.0f;
}

float ImGui_ImplWin32_GetDpiScaleForHwnd(HWND ahWnd)
{
    HMONITOR monitor = ::MonitorFromWindow(ahWnd, MONITOR_DEFAULTTONEAREST);
    return ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);
}

//---------------------------------------------------------------------------------------------------------
