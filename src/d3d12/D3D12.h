#pragma once

#include <window/Window.h>

using TCRenderNode_Present_InternalPresent = void(uint32_t*, uint8_t, uint32_t);
using TCRenderGlobal_Resize = void(uint32_t, uint32_t, uint32_t, uint8_t, uint32_t*);

struct D3D12
{
    inline static const uint32_t g_numDownlevelBackbuffersRequired = 3; // Windows 7 only: number of buffers needed before we start rendering

    D3D12(Window& aWindow, Paths& aPaths, Options& aOptions);
    ~D3D12();

    void ReloadFonts(bool aForce = false);

    void SetTrapInputInImGui(const bool acEnabled);
    void DelayedSetTrapInputInImGui(const bool acEnabled);
    [[nodiscard]] bool IsTrapInputInImGui() const noexcept { return m_trapInputInImGui; }
    [[nodiscard]] bool IsInitialized() const noexcept { return m_initialized; }
    [[nodiscard]] ImVec2 GetResolution() const noexcept { return m_resolution; }

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam) const;

    TiltedPhoques::Signal<void()> OnInitialize;

    std::tuple<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> CreateTextureDescriptor();

protected:

    void Hook();

    void Shutdown();

    void Initialize(uint32_t aSwapChainDataId);
    bool InitializeImGui();

    void PrepareUpdate();
    void Update(uint32_t aSwapChainDataId);

    static void CRenderNode_Present_InternalPresent(uint32_t* aSwapChainDataId, uint8_t a2, uint32_t a3);
    static void CRenderGlobal_Resize(uint32_t aWidth, uint32_t aHeight, uint32_t a3, uint8_t a4, uint32_t* aSwapChainDataId);

private:

    Paths& m_paths;
    Window& m_window;
    Options& m_options;

    TCRenderNode_Present_InternalPresent* m_realInternalPresent{ nullptr };
    TCRenderGlobal_Resize* m_realInternalResize{ nullptr };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pd3dSrvDescHeap{ };

    struct FrameContext
    {
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
    };

    std::array<FrameContext, SwapChainData_BackBufferCount> m_frameContexts;

    uint32_t m_swapChainDataId;

    std::recursive_mutex m_statePresentMutex;
    std::recursive_mutex m_stateGameMutex;

    std::recursive_mutex m_imguiMutex;

    std::array<ImDrawData, 3> m_imguiDrawDataBuffers;
    ImGuiStyle m_imguiStyleReference{ };

    FontSettings m_fontSettings;
    std::atomic_bool m_delayedTrapInput{ false };
    std::atomic_bool m_delayedTrapInputState{ false };
    std::atomic_bool m_trapInputInImGui{ false };

    std::atomic<ImVec2> m_resolution;

    std::atomic_bool m_initialized{ false };
    std::atomic_bool m_shutdown{ false };
};
