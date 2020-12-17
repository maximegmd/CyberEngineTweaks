#include "Image.h"
#include <spdlog/spdlog.h>
#include <MinHook.h>
#include <sstream>
#include "Options.h"
#include "Pattern.h"

struct GamePropertyString
{
    union
    {
        char* pValue;
        char pbValue[0x10];
    };

    uint32_t unk10; // some reason we can't add this to the union above

    uint32_t length_flags;

    // TODO: locate the actual string get/set functions in the game and use those instead
    char* get()
    {
        // Game will try storing the value inside this struct if the length is smaller than 0x14/0x10
        // Otherwise value is relocated to some other addr, and length gets 0x40000000 added to it to signal that

        if (length_flags >= 0x40000000)
            return pValue;

        return pbValue;
    }
};
static_assert(offsetof(GamePropertyString, length_flags) == 0x14);

enum class GamePropertyType : uint8_t
{
    Boolean,
    Integer,
    Float,
    String,
    Color // stored as int32
};

struct GameProperty
{
    virtual ~GameProperty();

    const char* pName;
    const char* pCategory;
    uint64_t unk18;
    uint64_t unk20;
    GamePropertyType type;
    uint8_t flag;
    uint8_t pad2A[0x30 - 0x2A];
    union
    {
        bool* pBoolean;
        uint32_t* pInteger;
        float* pFloat;
        GamePropertyString* pString;
    };

    std::string getInfo()
    {
        std::stringstream ret;

        if (pCategory)
            ret << pCategory << "/";

        if(pName)
            ret << pName;

        ret << " = ";

        switch (type)
        {
        case GamePropertyType::Boolean:
            if(pBoolean)
                ret << (*pBoolean ? "true" : "false");
            break;
        case GamePropertyType::Integer:
            if(pInteger)
                ret << *pInteger;
            break;
        case GamePropertyType::Float:
            if (pFloat)
                ret << *pFloat;
            break;
        case GamePropertyType::String:
            if (pString)
                ret << pString->get();
            break;
        case GamePropertyType::Color:
            if (pInteger)
                ret << "0x" << std::hex << *pInteger << std::dec;
            break;
        }

        return ret.str();
    }
};

static_assert(offsetof(GameProperty, type) == 0x28);
static_assert(offsetof(GameProperty, pBoolean) == 0x30);

bool HookGamePropertyGetBoolean(GameProperty* apThis, uint8_t* apVariable, GamePropertyType aType)
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

using TGamePropertyInit = void*(void*);
TGamePropertyInit* RealGamePropertyInit = nullptr;

// GameProperty pointers should remain valid for lifetime of the game
std::vector<GameProperty*> GameProperties;

void* HookGamePropertyInit(GameProperty* apThis)
{
    if (std::find(GameProperties.begin(), GameProperties.end(), apThis) == GameProperties.end())
    {
        GameProperties.push_back(apThis);
    }
    else
    {
        // GamePropertyInit seems to be called twice per property, value isn't set until after the first call though
        // Since we've already seen this property once we can now grab the value

        spdlog::info(apThis->getInfo());
    }

    return RealGamePropertyInit(apThis);
}

void OptionsInitPatch(Image* apImage)
{
    void* GamePropertyInit = FindSignature(apImage->pTextStart, apImage->pTextEnd,
        { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x57,
          0x48, 0x83, 0xEC, 0x40, 0x48, 0x8B, 0xF1, 0x48, 0x8D, 0x4C, 0x24, 0x20,
          0xE8 });

    if (GamePropertyInit)
    {
        MH_CreateHook(GamePropertyInit, &HookGamePropertyInit, reinterpret_cast<void**>(&RealGamePropertyInit));
        MH_EnableHook(GamePropertyInit);

        spdlog::info("\tHidden options hook: success");
    }
    else
        spdlog::info("\tHidden options hook: failed");
}
