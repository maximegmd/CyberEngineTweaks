#include "Image.h"
#include <spdlog/spdlog.h>
#include <mhook-lib/mhook.h>
#include <d3d11.h>
#include <atlbase.h>

using TRegisterPoolOptions = void(void*, const char*, uint64_t);
TRegisterPoolOptions* RealRegisterPoolOptions = nullptr;

uint64_t GetGPUMemory()
{
    const auto createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    CComPtr<ID3D11Device> device;
    CComPtr<ID3D11DeviceContext> context;

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_10_0;

    auto result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevels, std::size(featureLevels),
        D3D11_SDK_VERSION, &device, &level, &context);

    if (!device)
        return 0;

    CComQIPtr<IDXGIDevice> dxgiDevice = device.p;
    if (!dxgiDevice)
        return 0;

    CComPtr<IDXGIAdapter> adapter;
    if(FAILED(dxgiDevice->GetAdapter(&adapter)))
        return 0;

    DXGI_ADAPTER_DESC adapterDesc;
    if(FAILED(adapter->GetDesc(&adapterDesc)))
        return 0;

    return adapterDesc.DedicatedVideoMemory;
}

void RegisterPoolOptions(void* apThis, const char* acpName, uint64_t aSize)
{
    if (strcmp(acpName, "PoolCPU") == 0)
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);

        if (statex.ullTotalPhys)
        {
            const auto gigsInstalled = statex.ullTotalPhys >> 30;
            aSize = (gigsInstalled - 4) << 30;

            spdlog::info("\t\tDetected RAM: {}GB, using {}GB", gigsInstalled, aSize >> 30);
        }
    }
    else if (strcmp(acpName, "PoolGPU") == 0)
    {
        const auto returnedGpuMemory = GetGPUMemory();
        const auto fourGigs = 2ull << 30; // Assume at least 2 gigs of vram is available when we don't know
        const auto detectedGpuMemory = std::max(returnedGpuMemory, fourGigs);
        aSize = std::max(aSize, detectedGpuMemory);

        spdlog::info("\t\tUsing {}GB of VRAM", aSize >> 30);
    }

    RealRegisterPoolOptions(apThis, acpName, aSize);
}

void PoolPatch(Image* apImage)
{
    if (apImage->version == Image::MakeVersion(1, 4))
        RealRegisterPoolOptions = reinterpret_cast<TRegisterPoolOptions*>(0x1AD0F0 + apImage->base_address);

    if (RealRegisterPoolOptions)
    {
        auto result = Mhook_SetHook(reinterpret_cast<PVOID*>(&RealRegisterPoolOptions), &RegisterPoolOptions);
        spdlog::info("\tPool patch: {}", result ? "success":"error");
    }
    else
        spdlog::info("\tPool patch: failed");
}
