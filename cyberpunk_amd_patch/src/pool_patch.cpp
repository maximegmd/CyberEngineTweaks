#include "Image.h"
#include <spdlog/spdlog.h>
#include <mhook-lib/mhook.h>

using TRegisterPoolOptions = void(void*, const char*, uint64_t);
TRegisterPoolOptions* RealRegisterPoolOptions = nullptr;

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
        const auto fourGigs = 4ull << 30; // Assume at least 4 gigs of vram is available
        aSize = aSize > fourGigs ? aSize : fourGigs;

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
