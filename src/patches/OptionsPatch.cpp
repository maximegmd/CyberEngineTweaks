#include "CET.h"

#include <stdafx.h>

#include "Image.h"
#include "Options.h"
#include "scripting/GameOptions.h"

bool HookGameOptionGetBoolean(GameOption* apThis, uint8_t* apVariable, GameOptionType aType)
{
    auto* pVariable = apThis->pBoolean;
    if (!pVariable)
        return false;

    auto& options = CET::Get().GetOptions();
    if (options.PatchAsyncCompute && strcmp(apThis->pCategory, "Rendering/AsyncCompute") == 0)
    {
        *pVariable = false;
    }
    else if (options.PatchAntialiasing && strcmp(apThis->pName, "Antialiasing") == 0)
    {
        *pVariable = false;
    }
    else if (options.PatchAntialiasing && strcmp(apThis->pName, "ScreenSpaceReflection") == 0)
    {
        *pVariable = false;
    }

    if (aType != apThis->type)
        return false;

    *apVariable = *pVariable;

    return true;
}

void OptionsPatch(const Image* apImage)
{
    const mem::pattern cPattern("44 3A 41 28 75 11 48 8B 41 30 48 85 C0");
    const mem::default_scanner cScanner(cPattern);
    auto pLocation = cScanner(apImage->TextRegion).as<uint8_t*>();

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
        spdlog::warn("Hidden options patch: failed");
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

        if (CET::Get().GetOptions().DumpGameOptions)
            spdlog::info(apThis->GetInfo());
    }

    return RealGameOptionInit(apThis);
}

void OptionsInitHook(const Image* apImage)
{
    const mem::pattern cPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B F1 48 8D 4C 24 20 E8");
    const mem::default_scanner cScanner(cPattern);
    auto GameOptionInit = cScanner(apImage->TextRegion).as<uint8_t*>();

    if (GameOptionInit)
    {
        MH_CreateHook(GameOptionInit, &HookGameOptionInit, reinterpret_cast<void**>(&RealGameOptionInit));
        MH_EnableHook(GameOptionInit);

        spdlog::info("Hidden options hook: success");
    }
    else
        spdlog::warn("Hidden options hook: failed");
}
