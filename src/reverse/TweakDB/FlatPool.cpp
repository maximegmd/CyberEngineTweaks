#include "FlatPool.h"

FlatPool::FlatPool()
    : FlatPool(RED4ext::TweakDB::Get())
{
}

FlatPool::FlatPool(RED4ext::TweakDB* aTweakDb)
    : m_tweakDb(aTweakDb)
    , m_bufferEnd(0)
    , m_offsetEnd(0)
{
    m_pools.reserve(26);
    m_pools.emplace("Int32", 0);
    m_pools.emplace("Float", 0);
    m_pools.emplace("Bool", 0);
    m_pools.emplace("String", 0);
    m_pools.emplace("CName", 0);
    m_pools.emplace("TweakDBID", 0);
    m_pools.emplace("gamedataLocKeyWrapper", 0);
    m_pools.emplace("raRef:CResource", 0);
    m_pools.emplace("Quaternion", 0);
    m_pools.emplace("EulerAngles", 0);
    m_pools.emplace("Vector3", 0);
    m_pools.emplace("Vector2", 0);
    m_pools.emplace("Color", 0);
    m_pools.emplace("array:Int32", 0);
    m_pools.emplace("array:Float", 0);
    m_pools.emplace("array:Bool", 0);
    m_pools.emplace("array:String", 0);
    m_pools.emplace("array:CName", 0);
    m_pools.emplace("array:TweakDBID", 0);
    m_pools.emplace("array:gamedataLocKeyWrapper", 0);
    m_pools.emplace("array:raRef:CResource", 0);
    m_pools.emplace("array:Quaternion", 0);
    m_pools.emplace("array:EulerAngles", 0);
    m_pools.emplace("array:Vector3", 0);
    m_pools.emplace("array:Vector2", 0);
    m_pools.emplace("array:Color", 0);
}

bool FlatPool::IsFlatType(RED4ext::CBaseRTTIType* aType) const
{
    return m_pools.contains(aType->GetName());
}

bool FlatPool::IsFlatType(RED4ext::CName aTypeName) const
{
    return m_pools.contains(aTypeName);
}

int32_t FlatPool::AllocateValue(const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue)
{
    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        Initialize();

    const auto poolKey = aType->GetName();
    auto poolIt = m_pools.find(poolKey);

    if (poolIt == m_pools.end())
        return InvalidOffset;

    FlatValueMap& pool = poolIt.value();

    const auto hash = Hash(aType, aValue);
    const auto offsetIt = pool.find(hash);

    int32_t offset;

    if (offsetIt == pool.end())
    {
        offset = m_tweakDb->CreateFlatValue({const_cast<RED4ext::CBaseRTTIType*>(aType), aValue});

        if (offset != InvalidOffset)
            pool.emplace(hash, offset);

        SyncBuffer();
    }
    else
    {
        offset = offsetIt->second;
    }

    return offset;
}

int32_t FlatPool::AllocateData(const RED4ext::CStackType& aData)
{
    return AllocateValue(aData.type, aData.value);
}

int32_t FlatPool::AllocateDefault(const RED4ext::CBaseRTTIType* aType)
{
    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        Initialize();

    const auto typeKey = aType->GetName();
    auto offsetIt = m_defaults.find(typeKey);

    int32_t offset;

    if (offsetIt == m_defaults.end())
    {
        auto value = aType->GetAllocator()->AllocAligned(aType->GetSize(), aType->GetAlignment());
        std::memset(value.memory, 0, value.size);
        aType->Construct(value.memory);

        offset = AllocateValue(aType, value.memory);

        aType->Destruct(value.memory);
        aType->GetAllocator()->Free(value);

        if (offset != InvalidOffset)
        {
            m_defaults.emplace(typeKey, offset);
            SyncBuffer();
        }
    }
    else
    {
        offset = offsetIt->second;
    }

    return offset;
}

RED4ext::CStackType FlatPool::GetData(int32_t aOffset)
{
    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        Initialize();

    return GetFlatData(aOffset);
}

RED4ext::ScriptInstance FlatPool::GetValuePtr(int32_t aOffset)
{
    if (m_bufferEnd != m_tweakDb->flatDataBufferEnd)
        Initialize();

    return GetFlatData(aOffset).value;
}

void FlatPool::Initialize()
{
    const uintptr_t offsetEnd = m_tweakDb->flatDataBufferEnd - m_tweakDb->flatDataBuffer;

    if (m_offsetEnd == offsetEnd)
    {
        SyncBuffer();
        return;
    }

    std::shared_lock flatLockR(m_tweakDb->mutex00);

    constexpr auto FlatVFTSize = 8u;
    constexpr auto FlatAlignment = 8u;

    auto offset = RED4ext::AlignUp(static_cast<uint32_t>(m_offsetEnd), FlatAlignment);
    while (offset < offsetEnd)
    {
        // The current offset should always point to the VFT of the next flat.
        // If there's zero instead, that means the next value is 16-byte aligned,
        // and we need to skip the 8-byte padding to get to the flat.
        if (*reinterpret_cast<uint64_t*>(m_tweakDb->flatDataBuffer + offset) == 0ull)
            offset += 8u;

        const auto data = GetFlatData(static_cast<int32_t>(offset));
        const auto poolKey = data.type->GetName();

        auto poolIt = m_pools.find(poolKey);

        if (poolIt == m_pools.end())
            poolIt = m_pools.emplace(poolKey, 0).first;

        FlatValueMap& pool = poolIt.value();

        const auto hash = Hash(data.type, data.value);

        // Check for duplicates...
        // (Original game's blob has ~24K duplicates)
        if (!pool.contains(hash))
            pool.emplace(hash, offset);

        // Step {vft + data_size} aligned by {max(data_align, 8)}
        offset += RED4ext::AlignUp(FlatVFTSize + data.type->GetSize(), std::max(FlatAlignment, data.type->GetAlignment()));
    }

    SyncBuffer();
}

RED4ext::CStackType FlatPool::GetFlatData(int32_t aOffset)
{
    // This method uses VFTs to determine the flat type.
    // It's up to 33% faster than calling GetValue() every time.

    const auto addr = m_tweakDb->flatDataBuffer + aOffset;
    const auto vft = *reinterpret_cast<uintptr_t*>(addr);
    const auto it = m_vfts.find(vft);

    // For a known VFT we can immediately get RTTI type and data pointer.
    if (it != m_vfts.end())
        return {it->second.type, reinterpret_cast<void*>(addr + it->second.offset)};

    // For an unknown VFT, we call the virtual GetValue() once to get the type.
    const auto data = reinterpret_cast<RED4ext::TweakDB::FlatValue*>(addr)->GetValue();

    // Add type info to the map.
    // In addition to the RTTI type, we also store the data offset considering alignment.
    // Quaternion is 16-byte aligned, so there is 8-byte padding between the VFT and the data:
    // [ 8B VFT ][ 8B PAD ][ 16B QUATERNION ]
    m_vfts.insert({vft, {data.type, std::max(data.type->GetAlignment(), 8u)}});

    return data;
}

uint64_t FlatPool::Hash(const RED4ext::CBaseRTTIType* aType, RED4ext::ScriptInstance aValue)
{
    // Case 1: Everything is processed as a sequence of bytes and passed to the hash function,
    //         except for an array of strings.
    // Case 2: Arrays of strings are different because of empty strings that don't produce any
    //         hashing value. Therefore hash will be equal for different arrays in cases like:
    //         [] == [""] == ["", ""]
    //         ["", "a", "b"] == ["a", "", "b"] == ["a", "b", ""]
    //         As a workaround, we hash the string length as part of the data.

    uint64_t hash;

    if (aType->GetType() == RED4ext::ERTTIType::Array)
    {
        auto* arrayType = reinterpret_cast<const RED4ext::CRTTIArrayType*>(aType);
        const auto* innerType = arrayType->GetInnerType();

        if (innerType->GetName() == "String")
        {
            const auto* array = static_cast<RED4ext::DynArray<RED4ext::CString>*>(aValue);
            hash = RED4ext::FNV1a64(reinterpret_cast<uint8_t*>(0), 0); // Initial seed
            for (uint32_t i = 0; i != array->size; ++i)
            {
                const auto* str = array->entries + i;
                const auto length = str->Length();
                hash = RED4ext::FNV1a64(reinterpret_cast<const uint8_t*>(&length), sizeof(length), hash);
                hash = RED4ext::FNV1a64(reinterpret_cast<const uint8_t*>(str->c_str()), length, hash);
            }
        }
        else
        {
            const auto* array = static_cast<RED4ext::DynArray<uint8_t>*>(aValue);
            hash = RED4ext::FNV1a64(array->entries, array->size * innerType->GetSize());
        }
    }
    else if (aType->GetName() == "String")
    {
        const auto* str = static_cast<RED4ext::CString*>(aValue);
        const auto* data = reinterpret_cast<const uint8_t*>(str->c_str());
        hash = RED4ext::FNV1a64(data, str->Length());
    }
    else
    {
        const auto* data = static_cast<const uint8_t*>(aValue);
        hash = RED4ext::FNV1a64(data, aType->GetSize());
    }

    return hash;
}

void FlatPool::SyncBuffer()
{
    m_bufferEnd = m_tweakDb->flatDataBufferEnd;
    m_offsetEnd = m_tweakDb->flatDataBufferEnd - m_tweakDb->flatDataBuffer;
}
