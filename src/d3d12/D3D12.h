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
    static void Initialize();
    static void Shutdown();
    static D3D12& Get();

    ~D3D12();
    
    void TrapInputInImGui(bool aEnabled)
    {
        int showCursorState;
        if (aEnabled)
            do { showCursorState = ShowCursor(TRUE); } while (showCursorState < 0);
        else
            do { showCursorState = ShowCursor(FALSE); } while (showCursorState >= 0);

        /*
        // TODO: this does not seem to help cursor not showing when this is called from inside LuaVM 
        if (m_trapInputInImGui != aEnabled)
        {
            static auto cursor = LoadCursor(nullptr, IDC_ARROW);
            HCURSOR newCursor = (aEnabled) ? (cursor) : (nullptr);
            SetClassLongPtr(Window::Get().GetWindow(), GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(newCursor));
            SetCursor(newCursor);
        }
        */

        m_trapInputInImGui = aEnabled;
    }
    
    SIZE GetResolution() const { return m_outSize; }

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    bool IsInitialized() const { return m_initialized; }

protected:
    
    void Hook();

    struct FrameContext
    {
        CComPtr<ID3D12CommandAllocator> CommandAllocator;
        CComPtr<ID3D12Resource> BackBuffer;
        CComPtr<ID3D12Resource> MainRenderTargetResource;
        D3D12_CPU_DESCRIPTOR_HANDLE MainRenderTargetDescriptor{ 0 };
    };

    bool ResetState();
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

    D3D12();

    TResizeBuffersD3D12* m_realResizeBuffersD3D12{ nullptr };
    TPresentD3D12* m_realPresentD3D12{ nullptr };
    TPresentD3D12Downlevel* m_realPresentD3D12Downlevel{ nullptr };
    TCreateCommittedResource* m_realCreateCommittedResource{ nullptr };
    TExecuteCommandLists* m_realExecuteCommandLists{ nullptr };
    
    bool m_initialized{ false };

    std::vector<FrameContext> m_frameContexts;
    std::vector<CComPtr<ID3D12Resource>> m_downlevelBackbuffers;
    CComPtr<IDXGISwapChain3> m_pdxgiSwapChain;
    CComPtr<ID3D12Device> m_pd3d12Device;
    CComPtr<ID3D12DescriptorHeap> m_pd3dRtvDescHeap;
    CComPtr<ID3D12DescriptorHeap> m_pd3dSrvDescHeap;
    CComPtr<ID3D12GraphicsCommandList> m_pd3dCommandList;
    CComPtr<ID3D12CommandQueue> m_pCommandQueue;
    uint32_t m_downlevelBufferIndex;
    
    SIZE m_outSize{ };
    
    bool m_trapInputInImGui{ false };
};
