#include <stdafx.h>

#include "Image.h"
#include "Options.h"
#include "Pattern.h"
#include "scripting/GameOptions.h"

bool HookGameOptionGetBoolean(GameOption* apThis, uint8_t* apVariable, GameOptionType aType)
{
    auto* pVariable = apThis->pBoolean;
    if (!pVariable)
        return false;

    if (Options::Get().PatchAsyncCompute && strcmp(apThis->pCategory, "Rendering/AsyncCompute") == 0)
    {
        *pVariable = false;
    }
    else if (Options::Get().PatchAntialiasing && strcmp(apThis->pName, "Antialiasing") == 0)
    {
        *pVariable = false;
    }
    else if (Options::Get().PatchAntialiasing && strcmp(apThis->pName, "ScreenSpaceReflection") == 0)
    {
        *pVariable = false;
    }

    if (aType != apThis->type)
        return false;

    *apVariable = *pVariable;

    return true;
}

void OptionsPatch(Image* apImage)
{
    uint8_t* pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd,
        { 0x44, 0x3A, 0x41, 0x28, 0x75, 0x11, 0x48, 0x8B, 0x41, 0x30, 0x48, 0x85, 0xC0 });

    if (pLocation)
    {
        DWORD oldProtect = 0;
        VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);

        void* pAddress = &HookGameOptionGetBoolean;

        // mov rax, HookSpin
        pLocation[0] = 0x48;
        pLocation[1] = 0xB8;
        std::memcpy(pLocation + 2, &pAddress, 8);

        // jmp rax
        pLocation[10] = 0xFF;
        pLocation[11] = 0xE0;
        VirtualProtect(pLocation, 32, oldProtect, nullptr);
        
        spdlog::info("Hidden options patch: success");
    }
    else
        spdlog::info("Hidden options patch: failed");
}

using TGameOptionInit = void*(void*);
TGameOptionInit* RealGameOptionInit = nullptr;

void* HookGameOptionInit(GameOption* apThis)
{
    auto& gameOptions = GameOptions::GetList();

    if (std::find(gameOptions.begin(), gameOptions.end(), apThis) == gameOptions.end())
    {
        gameOptions.push_back(apThis);
    }
    else
    {
        // GameOptionInit seems to be called twice per option, value isn't set until after the first call though
        // Since we've already seen this option once we can now grab the value

        if(Options::Get().DumpGameOptions)
            spdlog::info(apThis->GetInfo());
    }

    return RealGameOptionInit(apThis);
}

void OptionsInitHook(Image* apImage)
{
    void* GameOptionInit = FindSignature(apImage->pTextStart, apImage->pTextEnd,
        { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57,
          0x48, 0x83, 0xEC, 0x40, 0x48, 0x8B, 0xF1, 0x48, 0x8D, 0x4C, 0x24, 0x20,
          0xE8 });

    if (GameOptionInit)
    {
        MH_CreateHook(GameOptionInit, &HookGameOptionInit, reinterpret_cast<void**>(&RealGameOptionInit));
        MH_EnableHook(GameOptionInit);

        spdlog::info("Hidden options hook: success");
    }
    else
        spdlog::info("Hidden options hook: failed");
}
