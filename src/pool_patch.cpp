#include "Image.h"
#include <spdlog/spdlog.h>
#include <MinHook.h>
#include <d3d11.h>
#include <atlbase.h>
#include "Options.h"
#include "Pattern.h"

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
    const uint64_t kScaler = 1024 * 1024 * 1024;

    const auto& options = Options::Get();

    if (strcmp(acpName, "PoolCPU") == 0)
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);

        if (statex.ullTotalPhys)
        {
            const auto gigsInstalled = statex.ullTotalPhys / kScaler;
            aSize = (gigsInstalled * options.CPUMemoryPoolFraction) * kScaler;

            spdlog::info("\tCPU RAM: {}GB, using {:.2}GB, fraction: {}", gigsInstalled, float(aSize) / kScaler, options.CPUMemoryPoolFraction);
        }
    }
    else if (strcmp(acpName, "PoolGPU") == 0)
    {
        const auto returnedGpuMemory = static_cast<uint64_t>(GetGPUMemory() * double(options.GPUMemoryPoolFraction));
        const auto defaultMemory = 512ull * 1024 * 1024; // Assume at least 512MB of vram is available when we don't know
        const auto detectedGpuMemory = std::max(returnedGpuMemory, defaultMemory);
        aSize = std::max(aSize, detectedGpuMemory);

        spdlog::info("\tGPU VRAM: {:.2}GB, using {:.2}GB, fraction: {}", float(detectedGpuMemory) / kScaler, float(aSize) / kScaler, options.GPUMemoryPoolFraction);
    }

    RealRegisterPoolOptions(apThis, acpName, aSize);
}

void PoolPatch(Image* apImage)
{
    RealRegisterPoolOptions = reinterpret_cast<TRegisterPoolOptions*>(FindSignature(apImage->pTextStart, apImage->pTextEnd,
        { 0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x74,0x24,0x10,0x57,0x48,0x83,0xEC,0x20,0x49,0x8B,0xD8,0x48,0x8B,0xFA,0x8B,0xF1,0xE8, 0xCC, 0xCC, 0xCC, 0xCC,0x48 ,0x8B ,0xC8 ,0x4C }));

    if (RealRegisterPoolOptions)
    {
        const auto result = MH_CreateHook(RealRegisterPoolOptions, &RegisterPoolOptions, reinterpret_cast<void**>(&RealRegisterPoolOptions));
        spdlog::info("\tPool patch: {}", result ? "success":"error");
    }
    else
        spdlog::info("\tPool patch: failed");
}
