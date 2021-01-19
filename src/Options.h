#pragma once

#include "Image.h"

struct Options
{
    static void Initialize(HMODULE aModule);
    static Options& Get();
    
    bool IsCyberpunk2077() const noexcept;
    
    bool PatchEnableDebug{ false };
    bool PatchRemovePedestrians{ false };
    bool PatchAsyncCompute{ false };
    bool PatchAntialiasing{ false };
    bool PatchSkipStartMenu{ true };
    bool PatchDisableIntroMovies{ false };
    bool PatchDisableVignette{ false };
    bool PatchDisableBoundaryTeleport{ false };
    bool PatchDisableWin7Vsync{ false };

    bool DumpGameOptions{ false };
    bool Console{ true };
    int ConsoleKey{ VK_OEM_3 };
    int ConsoleChar{ 0 };
    float CPUMemoryPoolFraction{ 0.5f };
    float GPUMemoryPoolFraction{ 1.f };
    std::filesystem::path CETPath;
    std::filesystem::path RootPath;
    std::filesystem::path ScriptsPath;
    std::string FontPath;
    std::string FontGlyphRanges{""};
    float FontSize{ 13.0f };
    Image GameImage;
    bool ExeValid{ false };

private:

    Options(HMODULE aModule);
};