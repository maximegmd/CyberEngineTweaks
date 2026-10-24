#pragma once

#ifdef __APPLE__
// CET macOS: the game window and its input (src/window/window_mac.mm). On Windows this class subclasses the game's WndProc; on
// macOS it finds the game's NSWindow and content view on the AppKit thread, follows their size, and installs one NSEvent local monitor.
// The monitor turns keyboard and mouse events into MacInputEvent records for the game thread (D3D12::PrepareUpdate feeds them to ImGui
// and VKBindings) and, while the overlay traps input, keeps them from the game.

#include <mutex>
#include <vector>

struct D3D12;
struct VKBindings;

struct MacInputEvent
{
    enum class Type : uint8_t
    {
        KeyDown,
        KeyUp,
        Text,
        MouseMove,
        MouseDown,
        MouseUp,
        Wheel,
        FocusLost
    };

    Type EventType{Type::KeyDown};
    uint16_t VKCode{0};      // Windows virtual-key code (keys and mouse buttons), 0 if none
    int32_t ImGuiKeyCode{0}; // ImGuiKey, 0 if none
    int32_t MouseButton{0};  // 0 left, 1 right, 2 middle, 3 and 4 extra
    uint32_t Character{0};   // Text: one UTF-32 code point
    float X{0}, Y{0};        // MouseMove: pixels from the top-left of the game view
    float WheelX{0}, WheelY{0};
    bool Shift{false}, Ctrl{false}, Alt{false}, Super{false};
};

struct Window
{
    Window(VKBindings* apBindings, D3D12* apD3D12);
    ~Window();

    HWND GetWindow() const { return m_window; } // NSWindow*
    void* GetView() const { return m_view; }   // the NSView whose layer CET adds its overlay layer to

    POINT GetWndPos() const { return m_wndPos; }
    SIZE GetWndSize() const { return m_wndSize; }

    POINT GetClientPos() const { return m_clientPos; }
    SIZE GetClientSize() const { return m_clientSize; } // pixels (points * backing scale)
    float GetBackingScale() const { return m_backingScale; }

    bool IsInitialized() const { return m_initialized; }

    // Game thread: move the queued input to aOut (oldest first).
    void DrainInput(std::vector<MacInputEvent>& aOut);

    // AppKit thread (window_mac.mm): window and view metrics, event queue.
    void UpdateMetrics();
    bool OnEvent(void* apNSEvent);

private:
    std::atomic_bool m_initialized{false};

    HWND m_window{nullptr};
    void* m_view{nullptr};
    void* m_monitor{nullptr};
    void* m_observers{nullptr};

    POINT m_wndPos{};
    SIZE m_wndSize{};
    POINT m_clientPos{};
    SIZE m_clientSize{};
    float m_backingScale{1.0f};

    std::mutex m_queueLock;
    std::vector<MacInputEvent> m_queue;
    uint32_t m_modifierFlags{0};

    VKBindings* m_pBindings;
    D3D12* m_pD3D12;
};
#else

struct D3D12;
struct VKBindings;

struct Window
{
    Window(VKBindings* apBindings, D3D12* apD3D12);
    ~Window();

    HWND GetWindow() const { return m_hWnd; }

    POINT GetWndPos() const { return m_wndPos; }
    SIZE GetWndSize() const { return m_wndSize; }

    POINT GetClientPos() const { return m_clientPos; }
    SIZE GetClientSize() const { return m_clientSize; }

    bool IsInitialized() const { return m_initialized; }

protected:
    static LRESULT APIENTRY WndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

private:
    bool m_initialized{false};

    HWND m_hWnd{nullptr};
    WNDPROC m_wndProc{nullptr};

    POINT m_wndPos{};
    SIZE m_wndSize{};
    POINT m_clientPos{};
    SIZE m_clientSize{};

    VKBindings* m_pBindings;
    D3D12* m_pD3D12;
};
#endif
