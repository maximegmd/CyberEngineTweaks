#pragma once

#include <cstdint>

struct CRTTIBaseType;

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

    static CGameEngine* Get();

    int8_t unk0[0x258];
    CGameFramework* framework;
};