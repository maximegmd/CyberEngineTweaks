#pragma once

#include "Image.h"

struct Options
{
    static void Initialize();
    static void Shutdown();
    
    static void Load();
    static void Save();
    static void ResetToDefaults(); 
    
    static inline Image GameImage;
    static inline UINT OverlayKeyBind{ 0 };
    static inline bool PatchEnableDebug{ false };
    static inline bool PatchRemovePedestrians{ false };
    static inline bool PatchAsyncCompute{ false };
    static inline bool PatchAntialiasing{ false };
    static inline bool PatchSkipStartMenu{ false };
    static inline bool PatchDisableIntroMovies{ false };
    static inline bool PatchDisableVignette{ false };
    static inline bool PatchDisableBoundaryTeleport{ false };
    static inline bool PatchDisableWin7Vsync{ false };
    static inline bool DumpGameOptions{ false };
    static inline bool ExeValid{ false };
    static inline bool IsFirstLaunch{ false };

    static inline bool Initialized{ false };
};