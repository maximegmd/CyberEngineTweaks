#pragma once

constexpr size_t SwapChainData_BackBufferCount = 3;

struct ID3D12CommandQueueDownlevel;
struct RenderContext
{
    struct SwapChainData
    {
        Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
        uint8_t unk8[0x24];
        uint32_t backBufferIndex;
        uint8_t unk30[0x8];
        HWND pWindow;
        void* unk40; // some pointer, looks to be ComPtr
        uint64_t bufferFrameNum[SwapChainData_BackBufferCount]; // seems to correspond to number of last rendered frame to each buffer
        uint64_t currentFrameNum; // looks to be current frame number
        Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers[SwapChainData_BackBufferCount];
        D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViews[SwapChainData_BackBufferCount];
        void* unk98; // some pointer
        uint8_t unkA0[0x10];
    };

    RenderContext() = delete;
    ~RenderContext() = delete;

    static RenderContext* GetInstance() noexcept;

    uint8_t pad0[0xC97F38];

    SwapChainData pSwapChainData[32];

    uint8_t pad[0xED68B0 - (0xC97F38 + sizeof(pSwapChainData))];

    Microsoft::WRL::ComPtr<ID3D12Device> pDevice; // ED68B0
    void* unkED68B8; // some pointer, looks to be ComPtr, not nullptr only on Win7
    Microsoft::WRL::ComPtr<ID3D12CommandQueueDownlevel> pDirectCommandQueueDownlevel; // ED68C0, not nullptr only on Win7
    void* unkED68C8; // some pointer, looks to be ComPtr
    void* unkED68D0; // some pointer, looks to be ComPtr
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> pDirectCommandQueue; // ED68D8

    // TODO - there seems to be more data after!
};

static_assert(sizeof(RenderContext::SwapChainData) == 0xB0);

static_assert(offsetof(RenderContext, pSwapChainData) == 0xC97F38);

static_assert(offsetof(RenderContext, pDevice) == 0xED68B0);
static_assert(offsetof(RenderContext, pDirectCommandQueueDownlevel) == 0xED68C0);
static_assert(offsetof(RenderContext, pDirectCommandQueue) == 0xED68D8);

static_assert(sizeof(Microsoft::WRL::ComPtr<IDXGISwapChain4>) == sizeof(IDXGISwapChain4*));
static_assert(sizeof(Microsoft::WRL::ComPtr<ID3D12Device>) == sizeof(ID3D12Device*));
static_assert(sizeof(Microsoft::WRL::ComPtr<ID3D12CommandQueueDownlevel>) == sizeof(ID3D12CommandQueueDownlevel*));
static_assert(sizeof(Microsoft::WRL::ComPtr<ID3D12CommandQueue>) == sizeof(ID3D12CommandQueue*));
