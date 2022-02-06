#pragma once

struct RenderContext
{
    struct Device
    {
        IDXGISwapChain* pSwapChain;
        uint8_t pad8[0xB8 - 0x8];
    };

    RenderContext() = delete;
    ~RenderContext() = delete;

    static RenderContext* GetInstance() noexcept;

    uint8_t pad0[0xC97F38];
    Device devices[0x30]; // Count unknown, it is at least 0x20
    uint8_t pad[0xED65C0 - 0xC9A1B8];
    void* unkED65C0; // Some pointer, might be related to d3d12on7, I have never seen it being anything other than null
};

static_assert(sizeof(RenderContext::Device) == 0xB8);
static_assert(offsetof(RenderContext, devices) == 0xC97F38);
static_assert(offsetof(RenderContext, unkED65C0) == 0xED65C0);
