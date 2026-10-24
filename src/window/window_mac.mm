// CET macOS: the game window and its input. See window/window.h (Apple branch).
//
// Threads: AppKit (the main thread) owns the NSWindow, the view and the event monitor; CET's game-thread task (D3D12::PrepareUpdate)
// drains the queued input. When the game loop itself runs on the main thread both are the same thread, which is fine.

#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <QuartzCore/QuartzCore.h>

#pragma push_macro("nil")
#undef nil
#define BOOL WINBOOL // the Win32 BOOL of the shared headers, not Objective-C's
#include <stdafx.h>

#include "window.h"

#include <CET.h>
#include <MacShims.hpp>
#undef BOOL
#pragma pop_macro("nil")

#include <pthread.h>

namespace
{
// macOS virtual key code (Carbon kVK_*) to the Windows virtual-key code raw input reports on a US layout. Raw input reports the generic
// VK_SHIFT / VK_CONTROL / VK_MENU for both sides of a modifier, so the left and right macOS modifiers map to those too; Command maps to
// the Windows key.
uint16_t MacKeyToVK(uint16_t aKeyCode)
{
    switch (aKeyCode)
    {
    case kVK_ANSI_A: return 'A';
    case kVK_ANSI_B: return 'B';
    case kVK_ANSI_C: return 'C';
    case kVK_ANSI_D: return 'D';
    case kVK_ANSI_E: return 'E';
    case kVK_ANSI_F: return 'F';
    case kVK_ANSI_G: return 'G';
    case kVK_ANSI_H: return 'H';
    case kVK_ANSI_I: return 'I';
    case kVK_ANSI_J: return 'J';
    case kVK_ANSI_K: return 'K';
    case kVK_ANSI_L: return 'L';
    case kVK_ANSI_M: return 'M';
    case kVK_ANSI_N: return 'N';
    case kVK_ANSI_O: return 'O';
    case kVK_ANSI_P: return 'P';
    case kVK_ANSI_Q: return 'Q';
    case kVK_ANSI_R: return 'R';
    case kVK_ANSI_S: return 'S';
    case kVK_ANSI_T: return 'T';
    case kVK_ANSI_U: return 'U';
    case kVK_ANSI_V: return 'V';
    case kVK_ANSI_W: return 'W';
    case kVK_ANSI_X: return 'X';
    case kVK_ANSI_Y: return 'Y';
    case kVK_ANSI_Z: return 'Z';
    case kVK_ANSI_0: return '0';
    case kVK_ANSI_1: return '1';
    case kVK_ANSI_2: return '2';
    case kVK_ANSI_3: return '3';
    case kVK_ANSI_4: return '4';
    case kVK_ANSI_5: return '5';
    case kVK_ANSI_6: return '6';
    case kVK_ANSI_7: return '7';
    case kVK_ANSI_8: return '8';
    case kVK_ANSI_9: return '9';
    case kVK_ANSI_Grave: return VK_OEM_3;
    case kVK_ANSI_Minus: return VK_OEM_MINUS;
    case kVK_ANSI_Equal: return VK_OEM_PLUS;
    case kVK_ANSI_LeftBracket: return VK_OEM_4;
    case kVK_ANSI_RightBracket: return VK_OEM_6;
    case kVK_ANSI_Backslash: return VK_OEM_5;
    case kVK_ANSI_Semicolon: return VK_OEM_1;
    case kVK_ANSI_Quote: return VK_OEM_7;
    case kVK_ANSI_Comma: return VK_OEM_COMMA;
    case kVK_ANSI_Period: return VK_OEM_PERIOD;
    case kVK_ANSI_Slash: return VK_OEM_2;
    case kVK_ISO_Section: return VK_OEM_102;
    case kVK_Return: return VK_RETURN;
    case kVK_Tab: return VK_TAB;
    case kVK_Space: return VK_SPACE;
    case kVK_Delete: return VK_BACK;
    case kVK_Escape: return VK_ESCAPE;
    case kVK_ForwardDelete: return VK_DELETE;
    case kVK_Home: return VK_HOME;
    case kVK_End: return VK_END;
    case kVK_PageUp: return VK_PRIOR;
    case kVK_PageDown: return VK_NEXT;
    case kVK_Help: return VK_INSERT;
    case kVK_LeftArrow: return VK_LEFT;
    case kVK_RightArrow: return VK_RIGHT;
    case kVK_UpArrow: return VK_UP;
    case kVK_DownArrow: return VK_DOWN;
    case kVK_Shift:
    case kVK_RightShift: return VK_SHIFT;
    case kVK_Control:
    case kVK_RightControl: return VK_CONTROL;
    case kVK_Option:
    case kVK_RightOption: return VK_MENU;
    case kVK_Command: return VK_LWIN;
    case kVK_RightCommand: return VK_RWIN;
    case kVK_CapsLock: return VK_CAPITAL;
    case kVK_F1: return VK_F1;
    case kVK_F2: return VK_F2;
    case kVK_F3: return VK_F3;
    case kVK_F4: return VK_F4;
    case kVK_F5: return VK_F5;
    case kVK_F6: return VK_F6;
    case kVK_F7: return VK_F7;
    case kVK_F8: return VK_F8;
    case kVK_F9: return VK_F9;
    case kVK_F10: return VK_F10;
    case kVK_F11: return VK_F11;
    case kVK_F12: return VK_F12;
    case kVK_F13: return VK_F13;
    case kVK_F14: return VK_F14;
    case kVK_F15: return VK_F15;
    case kVK_F16: return VK_F16;
    case kVK_F17: return VK_F17;
    case kVK_F18: return VK_F18;
    case kVK_F19: return VK_F19;
    case kVK_F20: return VK_F20;
    case kVK_ANSI_Keypad0: return VK_NUMPAD0;
    case kVK_ANSI_Keypad1: return VK_NUMPAD1;
    case kVK_ANSI_Keypad2: return VK_NUMPAD2;
    case kVK_ANSI_Keypad3: return VK_NUMPAD3;
    case kVK_ANSI_Keypad4: return VK_NUMPAD4;
    case kVK_ANSI_Keypad5: return VK_NUMPAD5;
    case kVK_ANSI_Keypad6: return VK_NUMPAD6;
    case kVK_ANSI_Keypad7: return VK_NUMPAD7;
    case kVK_ANSI_Keypad8: return VK_NUMPAD8;
    case kVK_ANSI_Keypad9: return VK_NUMPAD9;
    case kVK_ANSI_KeypadDecimal: return VK_DECIMAL;
    case kVK_ANSI_KeypadMultiply: return VK_MULTIPLY;
    case kVK_ANSI_KeypadPlus: return VK_ADD;
    case kVK_ANSI_KeypadClear: return VK_NUMLOCK;
    case kVK_ANSI_KeypadDivide: return VK_DIVIDE;
    case kVK_ANSI_KeypadEnter: return VK_RETURN;
    case kVK_ANSI_KeypadMinus: return VK_SUBTRACT;
    case kVK_ANSI_KeypadEquals: return VK_OEM_PLUS;
    case kVK_VolumeUp: return VK_VOLUME_UP;
    case kVK_VolumeDown: return VK_VOLUME_DOWN;
    case kVK_Mute: return VK_VOLUME_MUTE;
    default: return 0;
    }
}

// macOS virtual key code to ImGuiKey (the table of imgui_impl_osx.mm).
ImGuiKey MacKeyToImGui(uint16_t aKeyCode)
{
    switch (aKeyCode)
    {
    case kVK_ANSI_A: return ImGuiKey_A;
    case kVK_ANSI_B: return ImGuiKey_B;
    case kVK_ANSI_C: return ImGuiKey_C;
    case kVK_ANSI_D: return ImGuiKey_D;
    case kVK_ANSI_E: return ImGuiKey_E;
    case kVK_ANSI_F: return ImGuiKey_F;
    case kVK_ANSI_G: return ImGuiKey_G;
    case kVK_ANSI_H: return ImGuiKey_H;
    case kVK_ANSI_I: return ImGuiKey_I;
    case kVK_ANSI_J: return ImGuiKey_J;
    case kVK_ANSI_K: return ImGuiKey_K;
    case kVK_ANSI_L: return ImGuiKey_L;
    case kVK_ANSI_M: return ImGuiKey_M;
    case kVK_ANSI_N: return ImGuiKey_N;
    case kVK_ANSI_O: return ImGuiKey_O;
    case kVK_ANSI_P: return ImGuiKey_P;
    case kVK_ANSI_Q: return ImGuiKey_Q;
    case kVK_ANSI_R: return ImGuiKey_R;
    case kVK_ANSI_S: return ImGuiKey_S;
    case kVK_ANSI_T: return ImGuiKey_T;
    case kVK_ANSI_U: return ImGuiKey_U;
    case kVK_ANSI_V: return ImGuiKey_V;
    case kVK_ANSI_W: return ImGuiKey_W;
    case kVK_ANSI_X: return ImGuiKey_X;
    case kVK_ANSI_Y: return ImGuiKey_Y;
    case kVK_ANSI_Z: return ImGuiKey_Z;
    case kVK_ANSI_0: return ImGuiKey_0;
    case kVK_ANSI_1: return ImGuiKey_1;
    case kVK_ANSI_2: return ImGuiKey_2;
    case kVK_ANSI_3: return ImGuiKey_3;
    case kVK_ANSI_4: return ImGuiKey_4;
    case kVK_ANSI_5: return ImGuiKey_5;
    case kVK_ANSI_6: return ImGuiKey_6;
    case kVK_ANSI_7: return ImGuiKey_7;
    case kVK_ANSI_8: return ImGuiKey_8;
    case kVK_ANSI_9: return ImGuiKey_9;
    case kVK_ANSI_Grave: return ImGuiKey_GraveAccent;
    case kVK_ANSI_Minus: return ImGuiKey_Minus;
    case kVK_ANSI_Equal: return ImGuiKey_Equal;
    case kVK_ANSI_LeftBracket: return ImGuiKey_LeftBracket;
    case kVK_ANSI_RightBracket: return ImGuiKey_RightBracket;
    case kVK_ANSI_Backslash: return ImGuiKey_Backslash;
    case kVK_ANSI_Semicolon: return ImGuiKey_Semicolon;
    case kVK_ANSI_Quote: return ImGuiKey_Apostrophe;
    case kVK_ANSI_Comma: return ImGuiKey_Comma;
    case kVK_ANSI_Period: return ImGuiKey_Period;
    case kVK_ANSI_Slash: return ImGuiKey_Slash;
    case kVK_Return: return ImGuiKey_Enter;
    case kVK_Tab: return ImGuiKey_Tab;
    case kVK_Space: return ImGuiKey_Space;
    case kVK_Delete: return ImGuiKey_Backspace;
    case kVK_Escape: return ImGuiKey_Escape;
    case kVK_ForwardDelete: return ImGuiKey_Delete;
    case kVK_Home: return ImGuiKey_Home;
    case kVK_End: return ImGuiKey_End;
    case kVK_PageUp: return ImGuiKey_PageUp;
    case kVK_PageDown: return ImGuiKey_PageDown;
    case kVK_Help: return ImGuiKey_Insert;
    case kVK_LeftArrow: return ImGuiKey_LeftArrow;
    case kVK_RightArrow: return ImGuiKey_RightArrow;
    case kVK_UpArrow: return ImGuiKey_UpArrow;
    case kVK_DownArrow: return ImGuiKey_DownArrow;
    case kVK_Shift: return ImGuiKey_LeftShift;
    case kVK_RightShift: return ImGuiKey_RightShift;
    case kVK_Control: return ImGuiKey_LeftCtrl;
    case kVK_RightControl: return ImGuiKey_RightCtrl;
    case kVK_Option: return ImGuiKey_LeftAlt;
    case kVK_RightOption: return ImGuiKey_RightAlt;
    case kVK_Command: return ImGuiKey_LeftSuper;
    case kVK_RightCommand: return ImGuiKey_RightSuper;
    case kVK_CapsLock: return ImGuiKey_CapsLock;
    case kVK_F1: return ImGuiKey_F1;
    case kVK_F2: return ImGuiKey_F2;
    case kVK_F3: return ImGuiKey_F3;
    case kVK_F4: return ImGuiKey_F4;
    case kVK_F5: return ImGuiKey_F5;
    case kVK_F6: return ImGuiKey_F6;
    case kVK_F7: return ImGuiKey_F7;
    case kVK_F8: return ImGuiKey_F8;
    case kVK_F9: return ImGuiKey_F9;
    case kVK_F10: return ImGuiKey_F10;
    case kVK_F11: return ImGuiKey_F11;
    case kVK_F12: return ImGuiKey_F12;
    case kVK_ANSI_Keypad0: return ImGuiKey_Keypad0;
    case kVK_ANSI_Keypad1: return ImGuiKey_Keypad1;
    case kVK_ANSI_Keypad2: return ImGuiKey_Keypad2;
    case kVK_ANSI_Keypad3: return ImGuiKey_Keypad3;
    case kVK_ANSI_Keypad4: return ImGuiKey_Keypad4;
    case kVK_ANSI_Keypad5: return ImGuiKey_Keypad5;
    case kVK_ANSI_Keypad6: return ImGuiKey_Keypad6;
    case kVK_ANSI_Keypad7: return ImGuiKey_Keypad7;
    case kVK_ANSI_Keypad8: return ImGuiKey_Keypad8;
    case kVK_ANSI_Keypad9: return ImGuiKey_Keypad9;
    case kVK_ANSI_KeypadDecimal: return ImGuiKey_KeypadDecimal;
    case kVK_ANSI_KeypadMultiply: return ImGuiKey_KeypadMultiply;
    case kVK_ANSI_KeypadPlus: return ImGuiKey_KeypadAdd;
    case kVK_ANSI_KeypadClear: return ImGuiKey_NumLock;
    case kVK_ANSI_KeypadDivide: return ImGuiKey_KeypadDivide;
    case kVK_ANSI_KeypadEnter: return ImGuiKey_KeypadEnter;
    case kVK_ANSI_KeypadMinus: return ImGuiKey_KeypadSubtract;
    case kVK_ANSI_KeypadEquals: return ImGuiKey_KeypadEqual;
    default: return ImGuiKey_None;
    }
}

// The device-dependent modifier bit (IOKit NX_DEVICE*KEYMASK, low bits of modifierFlags) of a flagsChanged event's key code, so the
// left and right keys of one modifier are told apart; Caps Lock uses its toggle state. 0 for a key that is not a modifier.
NSEventModifierFlags ModifierFlagForKey(uint16_t aKeyCode)
{
    switch (aKeyCode)
    {
    case kVK_Control: return 0x00000001;
    case kVK_Shift: return 0x00000002;
    case kVK_RightShift: return 0x00000004;
    case kVK_Command: return 0x00000008;
    case kVK_RightCommand: return 0x00000010;
    case kVK_Option: return 0x00000020;
    case kVK_RightOption: return 0x00000040;
    case kVK_RightControl: return 0x00002000;
    case kVK_CapsLock: return NSEventModifierFlagCapsLock;
    default: return 0;
    }
}

NSView* FindMetalView(NSView* aView)
{
    if ([aView.layer isKindOfClass:[CAMetalLayer class]])
        return aView;
    for (NSView* child in aView.subviews)
    {
        if (NSView* found = FindMetalView(child))
            return found;
    }
    return (NSView*)nullptr;
}

// The game's window: the visible window whose view tree holds a CAMetalLayer (the game renders through one). Returns the window and
// the view that owns that layer.
bool FindGameView(NSWindow* __strong* apWindow, NSView* __strong* apView)
{
    for (NSWindow* window in NSApp.windows)
    {
        if (!window.visible || !window.contentView)
            continue;
        if (NSView* view = FindMetalView(window.contentView))
        {
            *apWindow = window;
            *apView = view;
            return true;
        }
    }
    return false;
}

void RunOnMain(void (^aBlock)())
{
    if (pthread_main_np())
        aBlock();
    else
        dispatch_async(dispatch_get_main_queue(), aBlock);
}
} // namespace

Window::Window(VKBindings* apBindings, D3D12* apD3D12)
    : m_pBindings(apBindings)
    , m_pD3D12(apD3D12)
{
    // As on Windows, look for the game window in the background until it exists (it does not at plugin load). The look-up itself runs
    // on the AppKit thread.
    std::thread t(
        [this]
        {
            using namespace std::chrono_literals;
            for (int attempt = 0; attempt < 6000 && !m_initialized; ++attempt) // 5 minutes at 50 ms
            {
                dispatch_sync(dispatch_get_main_queue(),
                              ^{
                                  if (m_initialized)
                                      return;
                                  NSWindow* window = nullptr;
                                  NSView* view = nullptr;
                                  if (!FindGameView(&window, &view))
                                      return;

                                  m_window = (__bridge_retained void*)window;
                                  m_view = (__bridge_retained void*)view;
                                  UpdateMetrics();

                                  NSEventMask mask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged | NSEventMaskMouseMoved |
                                                     NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp | NSEventMaskRightMouseDown |
                                                     NSEventMaskRightMouseUp | NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp |
                                                     NSEventMaskLeftMouseDragged | NSEventMaskRightMouseDragged |
                                                     NSEventMaskOtherMouseDragged | NSEventMaskScrollWheel;
                                  Window* self = this;
                                  id monitor = [NSEvent addLocalMonitorForEventsMatchingMask:mask
                                                                                     handler:^NSEvent*(NSEvent* aEvent) {
                                                                                         return self->OnEvent((__bridge void*)aEvent)
                                                                                                    ? (NSEvent*)nullptr
                                                                                                    : aEvent;
                                                                                     }];
                                  m_monitor = (__bridge_retained void*)monitor;

                                  NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
                                  NSMutableArray* observers = [NSMutableArray array];
                                  for (NSNotificationName name in @[
                                           NSWindowDidResizeNotification, NSWindowDidMoveNotification,
                                           NSWindowDidChangeBackingPropertiesNotification, NSWindowDidEnterFullScreenNotification,
                                           NSWindowDidExitFullScreenNotification
                                       ])
                                  {
                                      [observers addObject:[center addObserverForName:name
                                                                               object:window
                                                                                queue:nil
                                                                           usingBlock:^(NSNotification*) {
                                                                               self->UpdateMetrics();
                                                                           }]];
                                  }
                                  [observers addObject:[center addObserverForName:NSWindowDidResignKeyNotification
                                                                           object:window
                                                                            queue:nil
                                                                       usingBlock:^(NSNotification*) {
                                                                           std::lock_guard lock(self->m_queueLock);
                                                                           MacInputEvent focus;
                                                                           focus.EventType = MacInputEvent::Type::FocusLost;
                                                                           self->m_queue.push_back(focus);
                                                                       }]];
                                  m_observers = (__bridge_retained void*)observers;

                                  m_initialized = true;
                                  CetMac::Log("[cet] Window: game window found (%s, view %s, %dx%d px, scale %.1f); input monitor installed",
                                              window.className.UTF8String, view.className.UTF8String, m_clientSize.cx,
                                              m_clientSize.cy, m_backingScale);
                              });
                if (!m_initialized)
                    std::this_thread::sleep_for(50ms);
            }
            if (!m_initialized)
                CetMac::Log("[cet] Window: no game window with a CAMetalLayer after 5 minutes; the overlay stays off");
        });

    t.detach();
}

Window::~Window()
{
    void* monitor = m_monitor;
    void* observers = m_observers;
    void* window = m_window;
    void* view = m_view;
    m_monitor = m_observers = m_window = m_view = nullptr;
    m_initialized = false;
    if (!monitor && !observers && !window && !view)
        return;
    RunOnMain(^{
        if (monitor)
            [NSEvent removeMonitor:(__bridge_transfer id)monitor];
        if (observers)
        {
            NSArray* list = (__bridge_transfer NSArray*)observers;
            for (id observer in list)
                [[NSNotificationCenter defaultCenter] removeObserver:observer];
        }
        if (window)
            CFRelease(window);
        if (view)
            CFRelease(view);
    });
}

void Window::UpdateMetrics()
{
    NSWindow* window = (__bridge NSWindow*)m_window;
    NSView* view = (__bridge NSView*)m_view;
    if (!window || !view)
        return;

    const CGFloat scale = window.backingScaleFactor > 0 ? window.backingScaleFactor : 1.0;
    const NSRect frame = window.frame;
    const NSRect bounds = view.bounds;

    m_backingScale = static_cast<float>(scale);
    m_wndPos = {static_cast<LONG>(frame.origin.x), static_cast<LONG>(frame.origin.y)};
    m_wndSize = {static_cast<LONG>(frame.size.width * scale), static_cast<LONG>(frame.size.height * scale)};
    m_clientPos = {0, 0};
    m_clientSize = {static_cast<LONG>(bounds.size.width * scale), static_cast<LONG>(bounds.size.height * scale)};
}

bool Window::OnEvent(void* apNSEvent)
{
    NSEvent* event = (__bridge NSEvent*)apNSEvent;
    NSWindow* window = (__bridge NSWindow*)m_window;
    NSView* view = (__bridge NSView*)m_view;
    if (!event || !window || !view || (event.window && event.window != window))
        return false;

    const NSEventModifierFlags flags = event.modifierFlags;
    // While the overlay traps input, key presses, clicks, mouse moves and scrolling are kept from the game; Command shortcuts (Cmd+Q,
    // Cmd+Tab) never are. Releases (key up, modifier up, mouse up) always reach it: a key held when the overlay opened would otherwise
    // stay down in the game (W pressed as the overlay opened kept V walking).
    const bool trap = m_pD3D12 && m_pD3D12->IsTrapInputInImGui() && !(flags & NSEventModifierFlagCommand);
    auto fill = [&](MacInputEvent& aOut)
    {
        aOut.Shift = (flags & NSEventModifierFlagShift) != 0;
        aOut.Ctrl = (flags & NSEventModifierFlagControl) != 0;
        aOut.Alt = (flags & NSEventModifierFlagOption) != 0;
        aOut.Super = (flags & NSEventModifierFlagCommand) != 0;
    };

    std::lock_guard lock(m_queueLock);
    switch (event.type)
    {
    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
    {
        const bool down = event.type == NSEventTypeKeyDown;
        MacInputEvent key;
        key.EventType = down ? MacInputEvent::Type::KeyDown : MacInputEvent::Type::KeyUp;
        key.VKCode = MacKeyToVK(event.keyCode);
        key.ImGuiKeyCode = MacKeyToImGui(event.keyCode);
        fill(key);
        // a held key repeats keyDown; VKBindings ignores repeats itself, ImGui wants them for text fields
        m_queue.push_back(key);

        if (down && trap && !(flags & NSEventModifierFlagCommand))
        {
            NSString* characters = event.characters;
            for (NSUInteger i = 0; i < characters.length; ++i)
            {
                const unichar c = [characters characterAtIndex:i];
                if (c >= 0xF700 && c <= 0xF8FF) // function-key private-use range (arrows, F keys)
                    continue;
                if (c < 0x20 || c == 0x7F)
                    continue;
                MacInputEvent text;
                text.EventType = MacInputEvent::Type::Text;
                text.Character = c;
                m_queue.push_back(text);
            }
        }
        return down && trap;
    }
    case NSEventTypeFlagsChanged:
    {
        const NSEventModifierFlags bit = ModifierFlagForKey(event.keyCode);
        if (!bit)
            return trap;
        const bool down = (flags & bit) != 0;
        m_modifierFlags = static_cast<uint32_t>(flags);
        MacInputEvent key;
        key.EventType = down ? MacInputEvent::Type::KeyDown : MacInputEvent::Type::KeyUp;
        key.VKCode = MacKeyToVK(event.keyCode);
        key.ImGuiKeyCode = MacKeyToImGui(event.keyCode);
        fill(key);
        m_queue.push_back(key);
        return down && trap;
    }
    case NSEventTypeMouseMoved:
    case NSEventTypeLeftMouseDragged:
    case NSEventTypeRightMouseDragged:
    case NSEventTypeOtherMouseDragged:
    {
        const NSPoint inView = [view convertPoint:event.locationInWindow fromView:nil];
        const CGFloat height = view.bounds.size.height;
        MacInputEvent move;
        move.EventType = MacInputEvent::Type::MouseMove;
        move.X = static_cast<float>(inView.x * m_backingScale);
        move.Y = static_cast<float>((view.isFlipped ? inView.y : height - inView.y) * m_backingScale);
        fill(move);
        m_queue.push_back(move);
        return trap;
    }
    case NSEventTypeLeftMouseDown:
    case NSEventTypeLeftMouseUp:
    case NSEventTypeRightMouseDown:
    case NSEventTypeRightMouseUp:
    case NSEventTypeOtherMouseDown:
    case NSEventTypeOtherMouseUp:
    {
        const bool down = event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown ||
                          event.type == NSEventTypeOtherMouseDown;
        const NSInteger button = event.buttonNumber; // 0 left, 1 right, 2 middle, 3 and 4 extra
        if (button > 4)
            return trap;
        static constexpr uint16_t kButtonVK[] = {VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2};
        MacInputEvent click;
        click.EventType = down ? MacInputEvent::Type::MouseDown : MacInputEvent::Type::MouseUp;
        click.MouseButton = static_cast<int32_t>(button);
        click.VKCode = kButtonVK[button];
        fill(click);
        m_queue.push_back(click);
        return down && trap;
    }
    case NSEventTypeScrollWheel:
    {
        MacInputEvent wheel;
        wheel.EventType = MacInputEvent::Type::Wheel;
        CGFloat dx = event.scrollingDeltaX;
        CGFloat dy = event.scrollingDeltaY;
        if (event.hasPreciseScrollingDeltas)
        {
            dx *= 0.1;
            dy *= 0.1;
        }
        wheel.WheelX = static_cast<float>(dx);
        wheel.WheelY = static_cast<float>(dy);
        fill(wheel);
        if (wheel.WheelX != 0.0f || wheel.WheelY != 0.0f)
            m_queue.push_back(wheel);
        return trap;
    }
    default: return false;
    }
}

void Window::DrainInput(std::vector<MacInputEvent>& aOut)
{
    std::lock_guard lock(m_queueLock);
    aOut.insert(aOut.end(), m_queue.begin(), m_queue.end());
    m_queue.clear();
}

// TEST ONLY (Scripting.cpp registers the Lua function CetMacTestPostKey only when CP2077_AUTOTEST=1). Posts a key down and, aHoldMs later, a
// key up for the game window into AppKit's event queue ([NSApp postEvent:atStart:]): the game takes it from there as it takes a real key,
// so CET's local monitor sees it first.
void CetMacPostTestKey(uint16_t aKeyCode, int aHoldMs)
{
    const auto post = [](uint16_t aCode, bool aDown)
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            NSWindow* window = nullptr;
            NSView* view = nullptr;
            if (!FindGameView(&window, &view))
                return;
            NSEvent* event = [NSEvent keyEventWithType:aDown ? NSEventTypeKeyDown : NSEventTypeKeyUp
                                              location:NSZeroPoint
                                         modifierFlags:0
                                             timestamp:[[NSProcessInfo processInfo] systemUptime]
                                          windowNumber:window.windowNumber
                                               context:nil
                                            characters:@""
                           charactersIgnoringModifiers:@""
                                             isARepeat:NO
                                               keyCode:aCode];
            [NSApp postEvent:event atStart:NO];
        });
    };
    CetMac::Log("[cet] test key 0x%x posted (hold %d ms)", aKeyCode, aHoldMs);
    post(aKeyCode, true);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(std::max(aHoldMs, 1)) * NSEC_PER_MSEC), dispatch_get_main_queue(), ^{
        post(aKeyCode, false);
    });
}
