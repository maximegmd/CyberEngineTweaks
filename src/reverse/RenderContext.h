#pragma once

struct RenderContext
{
    struct Device
    {
        IDXGISwapChain* pSwapChain;
        uint8_t pad8[0x90 - 0x8];
    };

    RenderContext() = delete;
    ~RenderContext() = delete;

    static RenderContext* GetInstance() noexcept;

    uint8_t pad0[0xC97F38];
    Device devices[0x30]; // Count unknown, it is at least 0x20
    uint8_t pad[0xED64C0 - (0xC97F38 + sizeof(devices))];
    void* unkED69C0; // Some pointer, might be related to d3d12on7, I have never seen it being anything other than null
    uint8_t padED69C8[0x10];
    ID3D12CommandQueue* pDirectQueue; // ED64D8
};

static_assert(sizeof(RenderContext::Device) == 0x90);
static_assert(offsetof(RenderContext, devices) == 0xC97F38);
static_assert(offsetof(RenderContext, unkED69C0) == 0xED64C0);
static_assert(offsetof(RenderContext, pDirectQueue) == 0xED64D8);
