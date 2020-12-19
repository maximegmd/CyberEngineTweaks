#pragma once

#include <windows.h>
#include <cstddef>
#include <cstdint>

struct CRTTIBaseType;
struct SomeStruct;

struct BaseGameEngine
{
    virtual void sub_0() = 0;
};

struct CBaseEngine : BaseGameEngine
{
};

struct CGameEngine : CBaseEngine
{
    struct CGameFramework
    {
        struct Unk
        {
            virtual ~Unk() = 0;
            virtual uintptr_t GetTypeInstance(CRTTIBaseType* aClass) = 0;
        };

        int8_t unk0[0x10];
        Unk* unk10;
    };

    struct UnkC0
    {
        uint8_t pad0[0x140];
        uint32_t unk140;
        uint8_t pad144[0x164 - 0x144];
        uint32_t unk164;
        HWND Wnd;
        uint8_t pad170[0x9];
        uint8_t isClipped;
    };

    static CGameEngine* Get();

    int8_t pad8[0xC0 - 0x8];
    UnkC0* pSomeStruct;
    int8_t padC8[0x260 - 0xC8];
    CGameFramework* framework;
};

static_assert(offsetof(CGameEngine, pSomeStruct) == 0xC0);
static_assert(offsetof(CGameEngine, framework) == 0x260);

static_assert(offsetof(CGameEngine::UnkC0, unk140) == 0x140);
static_assert(offsetof(CGameEngine::UnkC0, unk164) == 0x164);
static_assert(offsetof(CGameEngine::UnkC0, Wnd) == 0x168);
static_assert(offsetof(CGameEngine::UnkC0, isClipped) == 0x179);