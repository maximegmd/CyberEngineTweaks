#include <stdafx.h>

#include <RED4ext/Types/TweakDB.hpp>

#include <reverse/WeakReference.h>
#include <reverse/StrongReference.h>
#include <scripting/Scripting.h>

#include "TweakDB.h"

struct FlatValuePool
{
    using HashType = uint32_t;

    enum class Type
    {
        Unknown = -1,
        ArrayTweakDBID,
        TweakDBID,
        ArrayQuaternion,
        Quaternion,
        ArrayEulerAngles,
        EulerAngles,
        ArrayVector3,
        Vector3,
        ArrayVector2,
        Vector2,
        ArrayColor,
        Color,
        ArrayGamedataLocKeyWrapper,
        GamedataLocKeyWrapper,
        ArrayRaRefCResource,
        RaRefCResource,
        ArrayCName,
        CName,
        ArrayBool,
        Bool,
        ArrayString,
        String,
        ArrayFloat,
        Float,
        ArrayInt32,
        Int32,
        Count
    };
    struct Item
    {
        HashType hash;
        uint32_t useCount;
        int32_t tdbOffset;

        Item(HashType aHash, int32_t aTDBOffset);
        void DecUseCount();
        void IncUseCount();
        RED4ext::TweakDB::FlatValue* ToFlatValue();
    };

    FlatValuePool(Type aType);
    Item* Get(const RED4ext::CStackType& acStackType);
    Item* Get(const RED4ext::CStackType& acStackType, HashType aHash);

    // Write-Lock TweakDB
    Item* GetOrCreate(const RED4ext::CStackType& acStackType, int32_t aTDBOffset = -1);
    // Write-Lock TweakDB
    Item* GetOrCreate(const RED4ext::CStackType& acStackType, HashType aHash, int32_t aTDBOffset);

    HashType HashValue(const RED4ext::CStackType& acStackType, HashType aSeed = HashType{});
    HashType HashValue(const RED4ext::CStackType& acStackType, Type aType, HashType aSeed = HashType{});

    static Type RTTIToPoolType(const RED4ext::IRTTIType* acpType);

private:
    Type poolType;
    std::vector<Item> items;
};
bool flatValuePoolsInitialized = false;
std::vector<FlatValuePool> flatValuePools;

void InitializeFlatValuePools()
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    if (flatValuePoolsInitialized) return;

    if (flatValuePools.empty())
    {
        flatValuePools.reserve(static_cast<int32_t>(FlatValuePool::Type::Count));
        for (int32_t i = 0; i != static_cast<int32_t>(FlatValuePool::Type::Count); ++i)
            flatValuePools.emplace_back(static_cast<FlatValuePool::Type>(i));
    }

    std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex00);
    if (pTDB->flatDataBufferCapacity == 0) return; // TweakDB is not initialized yet

    for (RED4ext::TweakDBID& flatID : pTDB->flats)
    {
        auto* pFlatValue = pTDB->GetFlatValue(flatID);

        RED4ext::CStackType stackType;
        pFlatValue->GetValue(&stackType);

        auto flatPoolType = FlatValuePool::RTTIToPoolType(stackType.type);
        if (flatPoolType == FlatValuePool::Type::Unknown)
        {
            flatValuePools.clear();

            RED4ext::CName typeName;
            stackType.type->GetName(typeName);
            auto exceptionStr = fmt::format("[InitializeFlatValuePools] Unknown flat type: {08X}:{02X} {}",
                flatID.nameHash, flatID.nameLength,
                typeName.ToString());
            throw std::exception(exceptionStr.c_str()); // This should never happen
        }

        auto* pFlatValuePool = &flatValuePools[static_cast<int32_t>(flatPoolType)];
        auto* pPoolItem = pFlatValuePool->GetOrCreate(stackType, pFlatValue->ToTDBOffset());
        pPoolItem->IncUseCount();
    }

    flatValuePoolsInitialized = true;
}

TweakDB::TweakDB(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aLua)
    : m_lua(aLua)
{
}

void TweakDB::DebugStats()
{
    static auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock<RED4ext::SharedMutex> _1(pTDB->mutex00);
    std::shared_lock<RED4ext::SharedMutex> _2(pTDB->mutex01);

    spdlog::get("scripting")->info("flats: {}", pTDB->flats.size);
    spdlog::get("scripting")->info("records: {}", pTDB->recordsByID.size);
    spdlog::get("scripting")->info("queries: {}", pTDB->queryIDs.size);
    spdlog::get("scripting")->info("flatDataBuffer capacity: {} bytes", pTDB->flatDataBufferCapacity);
}

sol::object TweakDB::GetRecord(TweakDBID aDBID)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    RED4ext::Handle<RED4ext::IScriptable> record;
    if (!pTDB->TryGetRecord(dbid, record))
        return sol::nil;

    auto state = m_lua.Lock();

    return make_object(state.Get(), StrongReference(m_lua, std::move(record)));
}

sol::object TweakDB::Query(TweakDBID aDBID)
{
    static auto* pRTTI = RED4ext::CRTTISystem::Get();
    static auto* pArrayTweakDBIDType = pRTTI->GetType("array:TweakDBID");
    static auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    RED4ext::DynArray<RED4ext::TweakDBID> queryResult;
    if (!pTDB->TryQuery(dbid, queryResult))
        return sol::nil;

    RED4ext::CStackType stackType;
    stackType.type = pArrayTweakDBIDType;
    stackType.value = &queryResult;
    auto state = m_lua.Lock();
    return Scripting::ToLua(state, stackType);
}

sol::object TweakDB::GetFlat(TweakDBID aDBID)
{
    static auto* pTDB = RED4ext::TweakDB::Get();
    //std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex00);

    RED4ext::TweakDBID dbid;
    dbid.value = aDBID.value;

    auto* flatValue = pTDB->GetFlatValue(dbid);
    if (flatValue == nullptr)
        return sol::nil;

    RED4ext::CStackType stackType;
    flatValue->GetValue(&stackType);
    auto state = m_lua.Lock();

    return Scripting::ToLua(state, stackType);
}

bool TweakDB::SetFlat(TweakDBID aDBID, sol::object aValue)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    InitializeFlatValuePools();

    RED4ext::TweakDBID* pDBID; // with tdbOffset
    {
        std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex00);

        RED4ext::TweakDBID tmpDBID;
        tmpDBID.value = aDBID.value;
        pDBID = std::find(pTDB->flats.begin(), pTDB->flats.end(), tmpDBID);
        if (pDBID == pTDB->flats.end())
        {
            return false;
        }
    }
    
    auto* flatValue = pTDB->GetFlatValue(*pDBID);
    if (flatValue == nullptr)
        return false;

    RED4ext::CStackType stackTypeCurrent;
    flatValue->GetValue(&stackTypeCurrent);

    auto flatPoolType = FlatValuePool::RTTIToPoolType(stackTypeCurrent.type);
    if (flatPoolType == FlatValuePool::Type::Unknown)
    {
        RED4ext::CName typeName;
        stackTypeCurrent.type->GetName(typeName);
        auto exceptionStr = fmt::format("[TweakDB::SetFlat] Unknown flat type: {08X}:{02X} {}",
            pDBID->nameHash, pDBID->nameLength,
            typeName.ToString());
        throw std::exception(exceptionStr.c_str()); // This should never happen
    }

    auto* pFlatValuePool = &flatValuePools[static_cast<int32_t>(flatPoolType)];
    auto* pCurentPoolItem = pFlatValuePool->Get(stackTypeCurrent);
    if (pCurentPoolItem == nullptr)
    {
        throw std::exception("[TweakDB::SetFlat] Couldn't find current item in pool"); // This should never happen
    }

    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 22);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    RED4ext::CStackType stackType = Scripting::ToRED(aValue, stackTypeCurrent.type, &s_scratchMemory);
    if (stackType.value == nullptr)
    {
        RED4ext::CName typeName;
        stackTypeCurrent.type->GetName(typeName);
        spdlog::get("scripting")->info("[TweakDB::SetFlat] Failed to convert value. Expecting: {}", typeName.ToString());
        return false;
    }
    if (stackType.type->IsEqual(stackType.value, stackTypeCurrent.value))
        return true;

    auto* pNewPoolItem = pFlatValuePool->GetOrCreate(stackType);
    if (pNewPoolItem == nullptr)
    {
        spdlog::get("scripting")->info("[TweakDB::SetFlat] Failed to create FlatValue");
        return false;
    }
    pCurentPoolItem->DecUseCount();
    pNewPoolItem->IncUseCount();

    pDBID->tdbOffsetBE[0] = reinterpret_cast<uint8_t*>(&pNewPoolItem->tdbOffset)[2];
    pDBID->tdbOffsetBE[1] = reinterpret_cast<uint8_t*>(&pNewPoolItem->tdbOffset)[1];
    pDBID->tdbOffsetBE[2] = reinterpret_cast<uint8_t*>(&pNewPoolItem->tdbOffset)[0];

    return true;
}

bool TweakDB::UpdateRecordByID(TweakDBID aDBID)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    return pTDB->UpdateRecord(RED4ext::TweakDBID(aDBID.value));
}

bool TweakDB::UpdateRecord(sol::object aValue)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    if (aValue.is<StrongReference>())
    {
        return pTDB->UpdateRecord(reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(aValue.as<StrongReference*>()->GetHandle()));
    }
    else if (aValue.is<WeakReference>())
    {
        return pTDB->UpdateRecord(reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(aValue.as<WeakReference*>()->GetHandle()));
    }
    else
    {
        spdlog::get("scripting")->info("[TweakDB::UpdateRecord] Expecting handle or whandle");
        return false;
    }
}

FlatValuePool::Item::Item(HashType aHash, int32_t aTDBOffset)
    : hash(aHash),
    useCount(0),
    tdbOffset(aTDBOffset)
{
}

void FlatValuePool::Item::DecUseCount()
{
    if (useCount == 0) return;
    --useCount;
}

void FlatValuePool::Item::IncUseCount()
{
    ++useCount;
}

RED4ext::TweakDB::FlatValue* FlatValuePool::Item::ToFlatValue()
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    int32_t offset = tdbOffset & 0x00FFFFFF;
    if (offset == 0)
        return nullptr;

    return reinterpret_cast<RED4ext::TweakDB::FlatValue*>(pTDB->flatDataBuffer + offset);
}

FlatValuePool::FlatValuePool(Type aType)
    : poolType(aType)
{
}

FlatValuePool::Item* FlatValuePool::Get(const RED4ext::CStackType& acStackType)
{
    return Get(acStackType, HashValue(acStackType));
}

FlatValuePool::Item* FlatValuePool::Get(const RED4ext::CStackType& acStackType, HashType aHash)
{
    for (auto& item : items)
    {
        if (item.hash != aHash) continue;

        const auto* pFlatValue = item.ToFlatValue();
        RED4ext::CStackType poolStackType;
        pFlatValue->GetValue(&poolStackType);

        if (poolStackType.type != acStackType.type)
            return nullptr; // This should never happen

        if (poolStackType.type->IsEqual(poolStackType.value, acStackType.value))
        {
            return &item;
        }
    }

    return nullptr;
}

FlatValuePool::Item* FlatValuePool::GetOrCreate(const RED4ext::CStackType& acStackType, int32_t aTDBOffset)
{
    return GetOrCreate(acStackType, HashValue(acStackType), aTDBOffset);
}

FlatValuePool::Item* FlatValuePool::GetOrCreate(const RED4ext::CStackType& acStackType, HashType aHash, int32_t aTDBOffset)
{
    static auto* pTDB = RED4ext::TweakDB::Get();

    auto* pItem = Get(acStackType, aHash);
    if (pItem != nullptr)
        return pItem;

    if (aTDBOffset == -1)
    {
        // Should we overwrite flat values with 0 useCount?
        // What happens to the records pointing to them that werent updated yet?

        auto* pFlatValue = pTDB->CreateFlatValue(acStackType);
        if (pFlatValue == nullptr)
        {
            // Failed to create FlatValue
            return nullptr;
        }
        return &items.emplace_back(Item(aHash, pFlatValue->ToTDBOffset()));
    }
    else
    {
        return &items.emplace_back(Item(aHash, aTDBOffset));
    }
}

FlatValuePool::HashType FlatValuePool::HashValue(const RED4ext::CStackType& acStackType, HashType aSeed)
{
    return HashValue(acStackType, poolType, aSeed);
}

FlatValuePool::HashType FlatValuePool::HashValue(const RED4ext::CStackType& acStackType, Type aType, HashType aSeed)
{
    switch (aType)
    {
        case FlatValuePool::Type::ArrayTweakDBID:
        case FlatValuePool::Type::ArrayQuaternion:
        case FlatValuePool::Type::ArrayEulerAngles:
        case FlatValuePool::Type::ArrayVector3:
        case FlatValuePool::Type::ArrayVector2:
        case FlatValuePool::Type::ArrayColor:
        case FlatValuePool::Type::ArrayGamedataLocKeyWrapper:
        case FlatValuePool::Type::ArrayRaRefCResource:
        case FlatValuePool::Type::ArrayCName:
        case FlatValuePool::Type::ArrayBool:
        case FlatValuePool::Type::ArrayString:
        case FlatValuePool::Type::ArrayFloat:
        case FlatValuePool::Type::ArrayInt32:
        {
            if (acStackType.type->GetType() != RED4ext::ERTTIType::Array)
            {
                RED4ext::CName typeName;
                acStackType.type->GetName(typeName);
                throw std::exception(fmt::format("[FlatValuePool::HashValue] Unexpected RTTI type: {}", typeName.ToString()).c_str());
            }

            const auto* pArrayType = reinterpret_cast<RED4ext::CArray*>(acStackType.type);
            auto* pArrayInnerType = pArrayType->GetInnerType();
            const auto arrayInnerPoolType = FlatValuePool::RTTIToPoolType(pArrayInnerType);
            if (arrayInnerPoolType == FlatValuePool::Type::Unknown)
            {
                RED4ext::CName typeName;
                pArrayInnerType->GetName(typeName);
                throw std::exception(fmt::format("[FlatValuePool::HashValue] Unexpected Array inner RTTI type: {}", typeName.ToString()).c_str());
            }

            HashType hash = aSeed;
            for (uint32_t i = 0; i != pArrayType->GetLength(acStackType.value); ++i)
            {
                RED4ext::CStackType innerStackType;
                innerStackType.type = pArrayInnerType;
                innerStackType.value = pArrayType->GetElement(acStackType.value, i);
                hash = HashValue(innerStackType, arrayInnerPoolType, hash);
            }

            return hash;
        }

        case FlatValuePool::Type::Quaternion:
        case FlatValuePool::Type::EulerAngles:
        case FlatValuePool::Type::Vector3:
        case FlatValuePool::Type::Vector2:
        case FlatValuePool::Type::Color:
        case FlatValuePool::Type::GamedataLocKeyWrapper:
        case FlatValuePool::Type::RaRefCResource:
        case FlatValuePool::Type::CName:
        case FlatValuePool::Type::Bool:
        case FlatValuePool::Type::Float:
        case FlatValuePool::Type::Int32:
        {
            auto* pData = reinterpret_cast<uint8_t*>(acStackType.value);
            return RED4ext::CRC32(pData, acStackType.type->GetSize(), aSeed);
        }

        case FlatValuePool::Type::TweakDBID:
        {
            auto* pData = reinterpret_cast<uint8_t*>(acStackType.value);
            return RED4ext::CRC32(pData, 4 + 1 /* nameHash + nameLen */, aSeed);
        }

        case FlatValuePool::Type::String:
        {
            auto* pString = reinterpret_cast<RED4ext::CString*>(acStackType.value);
            auto* pData = reinterpret_cast<const uint8_t*>(pString->c_str());
            return RED4ext::CRC32(pData, pString->Length(), aSeed);
        }
    }
    throw std::exception("[FlatValuePool::HashValue] Unknown PoolType"); // This should never happen
}

FlatValuePool::Type FlatValuePool::RTTIToPoolType(const RED4ext::IRTTIType* acpType)
{
    static auto* pRTTI = RED4ext::CRTTISystem::Get();
    static auto* pArrayTweakDBIDType = pRTTI->GetType("array:TweakDBID");
    static auto* pTweakDBIDType = pRTTI->GetType("TweakDBID");
    static auto* pArrayQuaternionType = pRTTI->GetType("array:Quaternion");
    static auto* pQuaternionType = pRTTI->GetType("Quaternion");
    static auto* pArrayEulerAnglesType = pRTTI->GetType("array:EulerAngles");
    static auto* pEulerAnglesType = pRTTI->GetType("EulerAngles");
    static auto* pArrayVector3Type = pRTTI->GetType("array:Vector3");
    static auto* pVector3Type = pRTTI->GetType("Vector3");
    static auto* pArrayVector2Type = pRTTI->GetType("array:Vector2");
    static auto* pVector2Type = pRTTI->GetType("Vector2");
    static auto* pArrayColorType = pRTTI->GetType("array:Color");
    static auto* pColorType = pRTTI->GetType("Color");
    static auto* pArrayGamedataLocKeyWrapperType = pRTTI->GetType("array:gamedataLocKeyWrapper");
    static auto* pGamedataLocKeyWrapperType = pRTTI->GetType("gamedataLocKeyWrapper");
    static auto* pArrayRaRefCResourceType = pRTTI->GetType("array:raRef:CResource");
    static auto* pRaRefCResourceType = pRTTI->GetType("raRef:CResource");
    static auto* pArrayCNameType = pRTTI->GetType("array:CName");
    static auto* pCNameType = pRTTI->GetType("CName");
    static auto* pArrayBoolType = pRTTI->GetType("array:Bool");
    static auto* pBoolType = pRTTI->GetType("Bool");
    static auto* pArrayStringType = pRTTI->GetType("array:String");
    static auto* pStringType = pRTTI->GetType("String");
    static auto* pArrayFloatType = pRTTI->GetType("array:Float");
    static auto* pFloatType = pRTTI->GetType("Float");
    static auto* pArrayInt32Type = pRTTI->GetType("array:Int32");
    static auto* pInt32Type = pRTTI->GetType("Int32");

    FlatValuePool::Type poolType = FlatValuePool::Type::Unknown;
    if (acpType == pArrayTweakDBIDType)
        poolType = FlatValuePool::Type::ArrayTweakDBID;
    else if (acpType == pTweakDBIDType)
        poolType = FlatValuePool::Type::TweakDBID;
    else if (acpType == pArrayQuaternionType)
        poolType = FlatValuePool::Type::ArrayQuaternion;
    else if (acpType == pQuaternionType)
        poolType = FlatValuePool::Type::Quaternion;
    else if (acpType == pArrayEulerAnglesType)
        poolType = FlatValuePool::Type::ArrayEulerAngles;
    else if (acpType == pEulerAnglesType)
        poolType = FlatValuePool::Type::EulerAngles;
    else if (acpType == pArrayVector3Type)
        poolType = FlatValuePool::Type::ArrayVector3;
    else if (acpType == pVector3Type)
        poolType = FlatValuePool::Type::Vector3;
    else if (acpType == pArrayVector2Type)
        poolType = FlatValuePool::Type::ArrayVector2;
    else if (acpType == pVector2Type)
        poolType = FlatValuePool::Type::Vector2;
    else if (acpType == pArrayColorType)
        poolType = FlatValuePool::Type::ArrayColor;
    else if (acpType == pColorType)
        poolType = FlatValuePool::Type::Color;
    else if (acpType == pArrayGamedataLocKeyWrapperType)
        poolType = FlatValuePool::Type::ArrayGamedataLocKeyWrapper;
    else if (acpType == pGamedataLocKeyWrapperType)
        poolType = FlatValuePool::Type::GamedataLocKeyWrapper;
    else if (acpType == pArrayRaRefCResourceType)
        poolType = FlatValuePool::Type::ArrayRaRefCResource;
    else if (acpType == pRaRefCResourceType)
        poolType = FlatValuePool::Type::RaRefCResource;
    else if (acpType == pArrayCNameType)
        poolType = FlatValuePool::Type::ArrayCName;
    else if (acpType == pCNameType)
        poolType = FlatValuePool::Type::CName;
    else if (acpType == pArrayBoolType)
        poolType = FlatValuePool::Type::ArrayBool;
    else if (acpType == pBoolType)
        poolType = FlatValuePool::Type::Bool;
    else if (acpType == pArrayStringType)
        poolType = FlatValuePool::Type::ArrayString;
    else if (acpType == pStringType)
        poolType = FlatValuePool::Type::String;
    else if (acpType == pArrayFloatType)
        poolType = FlatValuePool::Type::ArrayFloat;
    else if (acpType == pFloatType)
        poolType = FlatValuePool::Type::Float;
    else if (acpType == pArrayInt32Type)
        poolType = FlatValuePool::Type::ArrayInt32;
    else if (acpType == pInt32Type)
        poolType = FlatValuePool::Type::Int32;

    return poolType;
}
