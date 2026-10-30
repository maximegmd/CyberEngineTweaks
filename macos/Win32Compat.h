#pragma once

// CET macOS: the Win32 names CET's shared code uses for key codes and simple types, so VKBindings, the overlay widgets and the
// binding tables compile unchanged on macOS. Values are the ones in the Windows SDK (WinUser.h, winerror.h). Only types, constants and
// MacVKToChar live here: no Win32 function is emulated (the Apple branches call macOS directly).

#include <cstdint>
#include <cstring>
#include <strings.h>

// HMODULE, HANDLE, BOOL, DWORD, WORD, BYTE, UINT, WCHAR, LPVOID come from the SDK.
#include <RED4ext/Detail/WinCompat.hpp>

using USHORT = uint16_t;
using ULONG = uint32_t;
using LONG = int32_t;
using INT = int32_t;
using FLOAT = float;
using HRESULT = int32_t;
using LONG_PTR = intptr_t;
using UINT_PTR = uintptr_t;
using LRESULT = intptr_t;
using WPARAM = uintptr_t;
using LPARAM = intptr_t;
using HWND = void*; // on macOS: the game's NSWindow
using TCHAR = char;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef S_OK
#define S_OK ((HRESULT)0)
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

struct POINT
{
    LONG x;
    LONG y;
};

struct SIZE
{
    LONG cx;
    LONG cy;
};

struct RECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

inline int _stricmp(const char* a, const char* b)
{
    return strcasecmp(a, b);
}

inline int _strnicmp(const char* a, const char* b, size_t n)
{
    return strncasecmp(a, b, n);
}

#define TEXT(x) x

#define ZeroMemory(Destination, Length) std::memset((Destination), 0, (Length))

// MSVC byte-swap intrinsics (VKBindings encodes binds with them).
inline uint16_t _byteswap_ushort(uint16_t aValue)
{
    return __builtin_bswap16(aValue);
}
inline uint64_t _byteswap_uint64(uint64_t aValue)
{
    return __builtin_bswap64(aValue);
}

// Window messages CET's shared code names (the Mac input bridge does not send them; they keep switch statements compiling).
#define WM_SETFOCUS 0x0007
#define WM_KILLFOCUS 0x0008
#define WM_SETCURSOR 0x0020
#define WM_NCHITTEST 0x0084
#define WM_NCMOUSEMOVE 0x00A0
#define WM_INPUT 0x00FF
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_CHAR 0x0102
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP 0x020C
#define WM_XBUTTONDBLCLK 0x020D
#define WM_MOUSEHWHEEL 0x020E
#define WM_DEVICECHANGE 0x0219
#define WM_NCMOUSELEAVE 0x02A2
#define WM_MOUSELEAVE 0x02A3

// Raw input mouse flags (VKBindings encodes the wheel as a pseudo key from these).
#define RI_MOUSE_WHEEL 0x0400
#define RI_MOUSE_HWHEEL 0x0800

// Virtual-key codes (WinUser.h).
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_XBUTTON1 0x05
#define VK_XBUTTON2 0x06
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_KANA 0x15
#define VK_HANGUL 0x15
#define VK_JUNJA 0x17
#define VK_FINAL 0x18
#define VK_HANJA 0x19
#define VK_KANJI 0x19
#define VK_ESCAPE 0x1B
#define VK_CONVERT 0x1C
#define VK_NONCONVERT 0x1D
#define VK_ACCEPT 0x1E
#define VK_MODECHANGE 0x1F
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F
#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_SLEEP 0x5F
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_F13 0x7C
#define VK_F14 0x7D
#define VK_F15 0x7E
#define VK_F16 0x7F
#define VK_F17 0x80
#define VK_F18 0x81
#define VK_F19 0x82
#define VK_F20 0x83
#define VK_F21 0x84
#define VK_F22 0x85
#define VK_F23 0x86
#define VK_F24 0x87
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5
#define VK_BROWSER_BACK 0xA6
#define VK_BROWSER_FORWARD 0xA7
#define VK_BROWSER_REFRESH 0xA8
#define VK_BROWSER_STOP 0xA9
#define VK_BROWSER_SEARCH 0xAA
#define VK_BROWSER_FAVORITES 0xAB
#define VK_BROWSER_HOME 0xAC
#define VK_VOLUME_MUTE 0xAD
#define VK_VOLUME_DOWN 0xAE
#define VK_VOLUME_UP 0xAF
#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_STOP 0xB2
#define VK_MEDIA_PLAY_PAUSE 0xB3
#define VK_LAUNCH_MAIL 0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1 0xB6
#define VK_LAUNCH_APP2 0xB7
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_OEM_8 0xDF
#define VK_OEM_102 0xE2
#define VK_PROCESSKEY 0xE5
#define VK_PACKET 0xE7
#define VK_ATTN 0xF6
#define VK_CRSEL 0xF7
#define VK_EXSEL 0xF8
#define VK_EREOF 0xF9
#define VK_PLAY 0xFA
#define VK_ZOOM 0xFB
#define VK_NONAME 0xFC
#define VK_PA1 0xFD
#define VK_OEM_CLEAR 0xFE

// The character MapVirtualKey(vk, MAPVK_VK_TO_CHAR) gives on Windows with a US layout (VKBindings::GetBindString), 0 if none.
inline char MacVKToChar(uint16_t aVKCode)
{
    if ((aVKCode >= '0' && aVKCode <= '9') || (aVKCode >= 'A' && aVKCode <= 'Z'))
        return static_cast<char>(aVKCode);
    if (aVKCode >= VK_NUMPAD0 && aVKCode <= VK_NUMPAD9)
        return static_cast<char>('0' + (aVKCode - VK_NUMPAD0));
    switch (aVKCode)
    {
    case VK_SPACE: return ' ';
    case VK_MULTIPLY: return '*';
    case VK_ADD: return '+';
    case VK_SUBTRACT: return '-';
    case VK_DECIMAL: return '.';
    case VK_DIVIDE: return '/';
    case VK_OEM_1: return ';';
    case VK_OEM_PLUS: return '=';
    case VK_OEM_COMMA: return ',';
    case VK_OEM_MINUS: return '-';
    case VK_OEM_PERIOD: return '.';
    case VK_OEM_2: return '/';
    case VK_OEM_3: return '`';
    case VK_OEM_4: return '[';
    case VK_OEM_5: return '\\';
    case VK_OEM_6: return ']';
    case VK_OEM_7: return '\'';
    default: return 0;
    }
}
