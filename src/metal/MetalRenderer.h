#pragma once

// CET macOS: the Metal counterpart of src/d3d12/D3D12.h. The class keeps the name D3D12 so the shared code (CET.h, LuaVM,
// Scripting, Overlay, widgets, Window) is unchanged.
//
// Design (macos/PLATFORM_SURVEY.md): CET draws into its own transparent CAMetalLayer, a sublayer of the game view's layer, with ImGui's
// Metal backend on its own command queue. Nothing in the game's renderer or in Metal is hooked. The ImGui frame is built on the game
// thread (the same GameMainThread task as on Windows) and rendered right after it; the layer is hidden while nothing is drawn, so normal
// play pays nothing. Input comes from an NSEvent local monitor (src/window/window_mac.mm): events are queued on the AppKit thread and
// fed to ImGui and VKBindings on the game thread; while the overlay is open they are kept from the game.

#include "window/window.h"

struct D3D12
{
    D3D12(Window& aWindow, Paths& aPaths, Options& aOptions);
    ~D3D12();

    void ReloadFonts();

    void SetTrapInputInImGui(const bool acEnabled);
    void DelayedSetTrapInputInImGui(const bool acEnabled);
    [[nodiscard]] bool IsTrapInputInImGui() const noexcept { return m_trapInputInImGui; }
    [[nodiscard]] bool IsInitialized() const noexcept { return m_initialized; }
    [[nodiscard]] SIZE GetResolution() const noexcept { return m_outSize; }

    TiltedPhoques::Signal<void()> OnInitialized;

    // id<MTLDevice> (Texture.cpp).
    void* GetDevice() const;

    // The CET overlay layer and the ImGui frame counters, for logs and the self-test.
    [[nodiscard]] uint64_t GetFramesBuilt() const noexcept { return m_framesBuilt; }
    [[nodiscard]] uint64_t GetFramesPresented() const noexcept { return m_framesPresented; }

protected:
    bool Initialize();
    bool ResetState(const bool acDestroyContext = false);
    bool InitializeImGui();

    void PrepareUpdate();
    void Render();

private:
    bool m_initialized{false};
    bool m_initFailed{false};

    // Objective-C objects held through CFBridgingRetain (this header is plain C++).
    void* m_device{nullptr};       // id<MTLDevice>
    void* m_commandQueue{nullptr}; // id<MTLCommandQueue>
    void* m_layer{nullptr};        // CAMetalLayer*
    void* m_inFlight{nullptr};     // dispatch_semaphore_t, at most 2 frames in flight

    SIZE m_outSize{};
    std::atomic_bool m_layerVisible{false};
    std::atomic<uint64_t> m_framesBuilt{0};
    std::atomic<uint64_t> m_framesPresented{0};

    std::atomic_bool m_trapInputInImGui{false};
    ImGuiStyle m_styleReference{};

    Paths& m_paths;
    Window& m_window;
    Options& m_options;

    std::recursive_mutex m_imguiLock;
    std::atomic_bool m_delayedTrapInput{false};
    std::atomic_bool m_delayedTrapInputState{false};
};
