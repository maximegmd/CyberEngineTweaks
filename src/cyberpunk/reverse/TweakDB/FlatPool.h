#pragma once

#include <RED4ext/TweakDB.hpp>

class FlatPool
{
public:
    static constexpr int32_t InvalidOffset = -1;

    FlatPool();
    explicit FlatPool(RED4ext::TweakDB* aTweakDb);

    int32_t AllocateData(const RED4ext::CStackType& aData);
    int32_t AllocateValue(const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue);
    int32_t AllocateDefault(const RED4ext::CBaseRTTIType* aType);

    RED4ext::CStackType GetData(int32_t aOffset);
    RED4ext::ScriptInstance GetValuePtr(int32_t aOffset);

    [[nodiscard]] bool IsFlatType(RED4ext::CBaseRTTIType* aType) const;
    [[nodiscard]] bool IsFlatType(RED4ext::CName aTypeName) const;

private:
    struct FlatTypeInfo
    {
        RED4ext::CBaseRTTIType* type;
        uintptr_t offset;
    };

    using FlatValueMap = TiltedPhoques::Map<uint64_t, int32_t>;           // ValueHash -> Offset
    using FlatPoolMap = TiltedPhoques::Map<RED4ext::CName, FlatValueMap>; // TypeName -> Pool
    using FlatDefaultMap = TiltedPhoques::Map<RED4ext::CName, int32_t>;   // TypeName -> Offset
    using FlatTypeMap = TiltedPhoques::Map<uintptr_t, FlatTypeInfo>;      // VFT -> TypeInfo

    void Initialize();
    void SyncBuffer();

    inline RED4ext::CStackType GetFlatData(int32_t aOffset);
    inline static uint64_t Hash(const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue);

    RED4ext::TweakDB* m_tweakDb;
    uintptr_t m_bufferEnd;
    uintptr_t m_offsetEnd;
    FlatPoolMap m_pools;
    FlatDefaultMap m_defaults;
    FlatTypeMap m_vfts;
};
