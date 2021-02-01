#pragma once

#include "common/D3D12Downlevel.h"
#include "window/Window.h"

using TResizeBuffersD3D12 = HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
using TPresentD3D12 = HRESULT(IDXGISwapChain*, UINT, UINT);
using TPresentD3D12Downlevel = HRESULT(ID3D12CommandQueueDownlevel*, ID3D12GraphicsCommandList*, ID3D12Resource*, HWND, D3D12_DOWNLEVEL_PRESENT_FLAGS);
using TCreateCommittedResource = HRESULT(ID3D12Device*, const D3D12_HEAP_PROPERTIES*, D3D12_HEAP_FLAGS, const D3D12_RESOURCE_DESC*, D3D12_RESOURCE_STATES, const D3D12_CLEAR_VALUE*, const IID*, void**);
using TExecuteCommandLists = void(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

struct D3D12
{
    static const uint32_t g_numDownlevelBackbuffersRequired = 3; // Windows 7 only: number of buffers needed before we start rendering

    D3D12(Window& aWindow, Paths& aPaths, Options& aOptions);
    ~D3D12();
    
    void SetTrapInputInImGui(bool aEnabled);
    [[nodiscard]] bool IsTrapInputInImGui() const noexcept { return m_trapInputInImGui; }
    [[nodiscard]] bool IsInitialized() const noexcept { return m_initialized; }
    [[nodiscard]] SIZE GetResolution() const noexcept { return m_outSize; }

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    TiltedPhoques::Signal<void()> OnInitialized;
    TiltedPhoques::Signal<void()> OnUpdate;

protected:
    
    void Hook();

    struct FrameContext
    {
        CComPtr<ID3D12CommandAllocator> CommandAllocator { nullptr };
        CComPtr<ID3D12Resource> BackBuffer { nullptr };
        D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptor{ 0 };
    };

    bool ResetState(bool aClearDownlevelBackbuffers = true);
    bool Initialize(IDXGISwapChain* apSwapChain);
    bool InitializeDownlevel(ID3D12CommandQueue* apCommandQueue, ID3D12Resource* apSourceTex2D, HWND ahWindow);
    bool InitializeImGui(size_t aBuffersCounts);

    void Update();

    static HRESULT ResizeBuffers(IDXGISwapChain* apSwapChain, UINT aBufferCount, UINT aWidth, UINT aHeight, DXGI_FORMAT aNewFormat, UINT aSwapChainFlags);
    static HRESULT Present(IDXGISwapChain* apSwapChain, UINT aSyncInterval, UINT aPresentFlags);
    static HRESULT PresentDownlevel(ID3D12CommandQueueDownlevel* apCommandQueueDownlevel, ID3D12GraphicsCommandList* apOpenCommandList, ID3D12Resource* apSourceTex2D, HWND ahWindow, D3D12_DOWNLEVEL_PRESENT_FLAGS aFlags);
    static HRESULT CreateCommittedResource(ID3D12Device* apDevice, const D3D12_HEAP_PROPERTIES* acpHeapProperties, D3D12_HEAP_FLAGS aHeapFlags, const D3D12_RESOURCE_DESC* acpDesc, D3D12_RESOURCE_STATES aInitialResourceState, const D3D12_CLEAR_VALUE* acpOptimizedClearValue, const IID* acpRIID, void** appvResource);
    static void ExecuteCommandLists(ID3D12CommandQueue* apCommandQueue, UINT aNumCommandLists, ID3D12CommandList* const* apcpCommandLists);
    
private:

    TResizeBuffersD3D12* m_realResizeBuffersD3D12{ nullptr };
    TPresentD3D12* m_realPresentD3D12{ nullptr };
    TPresentD3D12Downlevel* m_realPresentD3D12Downlevel{ nullptr };
    TCreateCommittedResource* m_realCreateCommittedResource{ nullptr };
    TExecuteCommandLists* m_realExecuteCommandLists{ nullptr };
    
    bool m_initialized{ false };

    std::vector<FrameContext> m_frameContexts{ };
    std::vector<ID3D12Resource*> m_downlevelBackbuffers{ };
    IDXGISwapChain3* m_pdxgiSwapChain{ nullptr };
    CComPtr<ID3D12Device> m_pd3d12Device{ nullptr };
    CComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescHeap{ nullptr };
    CComPtr<ID3D12DescriptorHeap> m_pd3dSrvDescHeap{ nullptr };
    CComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList{ nullptr };
    ID3D12CommandQueue* m_pCommandQueue{ nullptr };
    uint32_t m_downlevelBufferIndex{ 0 };
    
    SIZE m_outSize{ };
    
    std::atomic_bool m_trapInputInImGui{ false };

    Paths& m_paths;
    Window& m_window;
    Options& m_options;
};
