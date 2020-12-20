#pragma once

#include <cstdint>

enum class RTTIType : uint8_t
{
    Name,
    Fundamental,
    Class,
    Array,
    Simple,
    Enum,
    StaticArray,
    NativeArray,
    Pointer,
    Handle,
    WeakHandle,
    ResourceReference,
    ResourceAsyncReference,
    BitField,
    LegacySingleChannelCurve,
    ScriptReference
};

struct CRTTIBaseType
{
    virtual ~CRTTIBaseType() = 0;
    virtual void GetName(uint64_t* aOut) = 0;
    virtual void sub_2() = 0;
    virtual void sub_3() = 0;
    virtual RTTIType GetType() = 0;

    int64_t unk8;
};

struct CClassFunction;
struct CBaseFunction
{
    uint8_t pad0[0x78];
    int32_t flags;
};

struct CClass : CRTTIBaseType
{
    CClassFunction* GetFunction(uint64_t aName);

    CClass* parent;
};

struct IRTTISystem
{
    virtual void sub_0() = 0;
    virtual void sub_1() = 0;
    virtual CRTTIBaseType* GetType(uint64_t aNameHash) = 0;
    virtual void sub_3() = 0;
    virtual void sub_4() = 0;
    virtual void sub_5() = 0;
    virtual CBaseFunction* GetGlobalFunction(uint64_t aNameHash) = 0;
    virtual void sub_7() = 0;
    virtual void sub_8() = 0;
    virtual void sub_9() = 0;
    virtual void sub_A() = 0;
    virtual void sub_B() = 0;
    virtual void sub_C() = 0;
    virtual void sub_D() = 0;
    virtual void sub_E() = 0;
    virtual void sub_F() = 0;
    virtual int64_t RegisterType(CRTTIBaseType* aType, uint32_t a2) = 0;
    virtual void sub_11() = 0;
    virtual void sub_12() = 0;
    virtual void RegisterFunction() = 0;
    virtual void sub_13() = 0;
    virtual void sub_14() = 0;
    virtual void sub_15() = 0;
    virtual void sub_16() = 0;
    virtual void sub_17() = 0;
    virtual void sub_18() = 0;
    virtual void sub_19() = 0;
    virtual void sub_1A() = 0;
    virtual void sub_1B() = 0;
    virtual void sub_1C() = 0;
    virtual void sub_1D() = 0;
    virtual void sub_1E() = 0;
    virtual void sub_1F() = 0;

    virtual ~IRTTISystem() = 0;

    template<typename T>
    T* GetType(uint64_t aNameHash)
    {
        return static_cast<T*>(GetType(aNameHash));
    }
};

struct CRTTISystem : IRTTISystem
{
    static CRTTISystem* Get();
};