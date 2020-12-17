#include "Image.h"
#include <spdlog/spdlog.h>
#include "Options.h"
#include "Pattern.h"

struct GameProperty
{
    virtual ~GameProperty();

    const char* pName;
    const char* pCategory;
    uint64_t unk18;
    uint64_t unk20;
    uint8_t kind;
    uint8_t type; // 2 is boolean
    uint8_t pad2A[0x30 - 0x2A];
    union
    {
        bool* pBoolean;
    };
};

static_assert(offsetof(GameProperty, kind) == 0x28);
static_assert(offsetof(GameProperty, pBoolean) == 0x30);

bool HookGamePropertyGetBoolean(GameProperty* apThis, uint8_t* apVariable, uint8_t aKind)
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

    if (aKind != apThis->kind)
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

        void* pAddress = &HookGamePropertyGetBoolean;

        // mov rax, HookSpin
        pLocation[0] = 0x48;
        pLocation[1] = 0xB8;
        std::memcpy(pLocation + 2, &pAddress, 8);

        // jmp rax
        pLocation[10] = 0xFF;
        pLocation[11] = 0xE0;
        VirtualProtect(pLocation, 32, oldProtect, nullptr);
        
        spdlog::info("\tHidden options patch: success");
    }
    else
        spdlog::info("\tHidden options patch: failed");
}
