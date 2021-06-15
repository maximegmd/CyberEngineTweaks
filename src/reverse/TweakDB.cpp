#include <stdafx.h>
#include <chrono>

#include <RED4ext/Types/TweakDB.hpp>

#include <CET.h>
#include <reverse/WeakReference.h>
#include <reverse/StrongReference.h>
#include <scripting/Scripting.h>

#include "TweakDB.h"

using namespace std::chrono_literals;

struct FlatPool
{
#define __HashFunction__ RED4ext::CRC32
    using HashType = decltype(__HashFunction__(nullptr, 0, 0));
    using THashFunc = HashType (*)(const uint8_t*, size_t, HashType);
    constexpr static HashType HashSeed = 0;
    constexpr static THashFunc HashFunc = __HashFunction__;
#undef __HashFunction__

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
        using clock = std::chrono::high_resolution_clock;

        // Too low may lead to race conditions
        // another thread using a record might get modified data
        // like a TweakDBID for vehicle record that was recycled to point to an item.
        // Give the user enough time to call TweakDB::UpdateRecord
        static constexpr auto c_recycleWaitTime = 10s;

        HashType m_hash;
        uint32_t m_useCount;
        int32_t m_tdbOffset;
        clock::time_point m_availableAt; // available for recycle

        Item(HashType aHash, int32_t aTDBOffset) noexcept;
        void DecUseCount();
        void IncUseCount();
        bool ReadyForRecycle();
        RED4ext::TweakDB::FlatValue* ToFlatValue();
    };

    // Increases m_useCount for 'FlatPool::Item'
    int32_t Get(const RED4ext::CStackType& acStackType);
    // Increases m_useCount for 'FlatPool::Item'
    int32_t Get(const RED4ext::CStackType& acStackType, HashType aHash);

    // Write-Lock TweakDB / Increases m_useCount for 'FlatPool::Item'
    int32_t Create(const RED4ext::CStackType& acStackType, int32_t aTDBOffset = -1);
    // Write-Lock TweakDB / Increases m_useCount for 'FlatPool::Item'
    int32_t Create(const RED4ext::CStackType& acStackType, HashType aHash, int32_t aTDBOffset);

    // Write-Lock TweakDB / Increases m_useCount for 'FlatPool::Item'
    int32_t GetOrCreate(const RED4ext::CStackType& acStackType, int32_t aTDBOffset = -1);
    // Write-Lock TweakDB / Increases m_useCount for 'FlatPool::Item'
    int32_t GetOrCreate(const RED4ext::CStackType& acStackType, HashType aHash, int32_t aTDBOffset);

    // Decreases m_useCount for 'FlatPool::Item'
    bool Remove(int32_t aTDBOffset);

    HashType HashValue(const RED4ext::CStackType& acStackType);
    HashType HashValue(const RED4ext::CStackType& acStackType, Type aType, HashType aSeed = HashSeed);

    static bool Initialize();
    static FlatPool* GetPool(const RED4ext::IRTTIType* acpType);
    static Type RTTIToPoolType(const RED4ext::IRTTIType* acpType);

private:
    std::mutex m_mutex;
    Type m_poolType = Type::Unknown;
    TiltedPhoques::Vector<Item> m_items;

    static bool s_initialized;
    static std::array<FlatPool, (size_t)Type::Count> s_pools;
};

std::mutex TweakDB::s_mutex;
std::set<TweakDBID> TweakDB::s_createdRecords;
bool FlatPool::s_initialized = false;
std::array<FlatPool, (size_t)FlatPool::Type::Count> FlatPool::s_pools;

TweakDB::TweakDB(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aLua)
    : m_lua(aLua)
{
}

void TweakDB::DebugStats()
{
    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock<RED4ext::SharedMutex> _1(pTDB->mutex00);
    std::shared_lock<RED4ext::SharedMutex> _2(pTDB->mutex01);
    auto logger = spdlog::get("scripting"); // DebugStats should always log to console

    logger->info("flats: {}", pTDB->flats.size);
    logger->info("records: {}", pTDB->recordsByID.size);
    logger->info("queries: {}", pTDB->queries.GetSize());
    size_t flatDataBufferSize = pTDB->flatDataBufferEnd - pTDB->flatDataBuffer;
    logger->info("flatDataBuffer: {}/{} bytes", flatDataBufferSize, pTDB->flatDataBufferCapacity);
    logger->info("created records: {}", s_createdRecords.size());
}

sol::object TweakDB::GetRecords(const std::string& acRecordTypeName)
{
    static auto* pArrayType = RED4ext::CRTTISystem::Get()->GetType("array:handle:IScriptable");
    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex01);

    auto* pRecordType = RED4ext::CRTTISystem::Get()->GetType(acRecordTypeName.c_str());
    if (pRecordType == nullptr)
        return sol::nil;

    auto* pRecords = pTDB->recordsByType.Get(pRecordType);
    if (pRecords == nullptr)
        return sol::nil;

    RED4ext::CStackType stackType(pArrayType, pRecords);
    auto state = m_lua.Lock();
    return Scripting::ToLua(state, stackType);
}

sol::object TweakDB::GetRecordByName(const std::string& acRecordName)
{
    return std::move(GetRecord(TweakDBID(acRecordName)));
}

sol::object TweakDB::GetRecord(TweakDBID aDBID)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::Handle<RED4ext::IScriptable> record;
    if (!pTDB->TryGetRecord(aDBID.value, record))
        return sol::nil;

    auto state = m_lua.Lock();
    return make_object(state.Get(), StrongReference(m_lua, std::move(record)));
}

sol::object TweakDB::QueryByName(const std::string& acQueryName)
{
    return std::move(Query(TweakDBID(acQueryName)));
}

sol::object TweakDB::Query(TweakDBID aDBID)
{
    static auto* pArrayTweakDBIDType = RED4ext::CRTTISystem::Get()->GetType("array:TweakDBID");
    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex01);

    RED4ext::DynArray<RED4ext::TweakDBID> queryResult;
    if (!pTDB->TryQuery(aDBID.value, queryResult))
        return sol::nil;

    RED4ext::CStackType stackType(pArrayTweakDBIDType, &queryResult);
    auto state = m_lua.Lock();
    return Scripting::ToLua(state, stackType);
}

sol::object TweakDB::GetFlatByName(const std::string& acFlatName)
{
    return std::move(GetFlat(TweakDBID(acFlatName)));
}

sol::object TweakDB::GetFlat(TweakDBID aDBID)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pFlatValue = pTDB->GetFlatValue(aDBID.value);
    if (pFlatValue == nullptr)
        return sol::nil;

    RED4ext::CStackType stackType = pFlatValue->GetValue();
    auto state = m_lua.Lock();
    return Scripting::ToLua(state, stackType);
}

bool TweakDB::SetFlatsByName(const std::string& acRecordName, sol::table aTable, sol::this_environment aThisEnv)
{
    return SetFlats(TweakDBID(acRecordName), std::move(aTable), std::move(aThisEnv));
}

bool TweakDB::SetFlats(TweakDBID aDBID, sol::table aTable, sol::this_environment aThisEnv)
{
    bool success = true;
    TweakDBID prepDBID(aDBID, ".");

    for (auto& [key, value] : aTable)
    {
        if (!key.is<std::string>())
            continue;

        TweakDBID flatDBID(prepDBID, key.as<std::string>());
        success &= SetFlat(flatDBID, value, aThisEnv);
    }

    UpdateRecordByID(aDBID);

    return success;
}

bool TweakDB::SetFlatByName(const std::string& acFlatName, sol::object aObject, sol::this_environment aThisEnv)
{
    return SetFlat(TweakDBID(acFlatName), std::move(aObject), std::move(aThisEnv));
}

bool TweakDB::SetFlat(TweakDBID aDBID, sol::object aObject, sol::this_environment aThisEnv)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 22);
    struct ResetAllocator
    {
        ~ResetAllocator()
        {
            s_scratchMemory.Reset();
        }
    };
    ResetAllocator ___allocatorReset;

    const sol::environment cEnv = aThisEnv;
    std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    RED4ext::CStackType stackType;
    {
        auto* pFlatValue = pTDB->GetFlatValue(aDBID.value);
        if (pFlatValue == nullptr)
            return false;

        RED4ext::CStackType stackTypeCurrent = pFlatValue->GetValue();
        stackType = Scripting::ToRED(aObject, stackTypeCurrent.type, &s_scratchMemory);
        if (stackType.value == nullptr)
        {
            RED4ext::CName typeName;
            stackTypeCurrent.type->GetName(typeName);
            logger->info("[TweakDB::SetFlat] Failed to convert value. Expecting: {}", typeName.ToString());
            return false;
        }
    }

    if (InternalSetFlat(aDBID.value, stackType) == -1)
    {
        logger->info("[TweakDB::SetFlat] Failed to create a new TweakDB Flat");
        return false;
    }

    return true;
}

bool TweakDB::SetFlatByNameAutoUpdate(const std::string& acFlatName, sol::object aObject,
                                      sol::this_environment aThisEnv)
{
    return SetFlatAutoUpdate(TweakDBID(acFlatName), std::move(aObject), std::move(aThisEnv));
}

bool TweakDB::SetFlatAutoUpdate(TweakDBID aDBID, sol::object aObject, sol::this_environment aThisEnv)
{
    if (SetFlat(aDBID, std::move(aObject), std::move(aThisEnv)))
    {
        uint64_t recordDBID = CET::Get().GetVM().GetTDBIDBase(aDBID.value);
        if (recordDBID != 0)
        {
            UpdateRecordByID(recordDBID);
        }
        return true;
    }

    return false;
}

bool TweakDB::UpdateRecordByName(const std::string& acRecordName)
{
    return UpdateRecordByID(TweakDBID(acRecordName));
}

bool TweakDB::UpdateRecordByID(TweakDBID aDBID)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    return pTDB->UpdateRecord(aDBID.value);
}

bool TweakDB::UpdateRecord(sol::object aValue, sol::this_environment aThisEnv)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    const sol::environment cEnv = aThisEnv;
    std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    RED4ext::gamedataTweakDBRecord* pRecord;
    if (aValue.is<StrongReference>())
    {
        pRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(aValue.as<StrongReference*>()->GetHandle());
    }
    else if (aValue.is<WeakReference>())
    {
        pRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(aValue.as<WeakReference*>()->GetHandle());
    }
    else
    {
        logger->info("[TweakDB::UpdateRecord] Expecting handle or whandle");
        return false;
    }

    return pTDB->UpdateRecord(pRecord);
}

bool TweakDB::CreateRecord(const std::string& acRecordName, const std::string& acRecordTypeName,
                           sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalCreateRecord(acRecordName, acRecordTypeName, logger);
}

bool TweakDB::CloneRecordByName(const std::string& acRecordName, const std::string& acClonedRecordName,
                                sol::this_environment aThisEnv)
{
    return CloneRecord(acRecordName, TweakDBID(acClonedRecordName), std::move(aThisEnv));
}

bool TweakDB::CloneRecord(const std::string& acRecordName, TweakDBID aClonedRecordDBID, sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalCloneRecord(acRecordName, aClonedRecordDBID.value, logger);
}

bool TweakDB::DeleteRecord(const std::string& acRecordName, sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    std::shared_ptr<spdlog::logger> logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalDeleteRecord(TweakDBID(acRecordName), logger);
}

int32_t TweakDB::InternalSetFlat(TweakDBID aDBID, const RED4ext::CStackType& acStackType)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pFlatValue = pTDB->GetFlatValue(aDBID);
    if (pFlatValue == nullptr)
        return -1;

    RED4ext::CStackType stackTypeCurrent = pFlatValue->GetValue();
    if (acStackType.type->IsEqual(acStackType.value, stackTypeCurrent.value))
        return pFlatValue->ToTDBOffset();

    FlatPool* pPool = FlatPool::GetPool(stackTypeCurrent.type);
    if (pPool == nullptr)
    {
        // This should never happen
        assert(false);
        return -1;
    }

    int32_t newTDBOffset = pPool->GetOrCreate(acStackType);
    if (newTDBOffset == -1)
        return -1;

    {
        std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex00);

        auto* pDBID = pTDB->flats.Find(aDBID);
        if (pDBID == pTDB->flats.end())
        {
            pPool->Remove(newTDBOffset);
            return -1;
        }

        pDBID->SetTDBOffset(newTDBOffset);
    }

    pPool->Remove(pFlatValue->ToTDBOffset());
    return newTDBOffset;
}

bool TweakDB::InternalCreateRecord(const std::string& acRecordName, const std::string& acRecordTypeName,
                                   std::shared_ptr<spdlog::logger> aLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pType = RED4ext::CRTTISystem::Get()->GetType(acRecordTypeName.c_str());
    RED4ext::Handle<RED4ext::IScriptable> record;
    {
        std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex01);

        RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> recordsOfSameType;
        if (!pTDB->TryGetRecordsByType(pType, recordsOfSameType) || recordsOfSameType.size == 0)
        {
            if (aLogger)
            {
                aLogger->info("Failed to create record '{}'. reason: Unknown type '{}'", acRecordName,
                              acRecordTypeName);
            }
            return false;
        }
        record = recordsOfSameType[0];
    }

    auto* pTweakRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(record.GetPtr());
    return InternalCloneRecord(acRecordName, pTweakRecord, false, aLogger);
}

bool TweakDB::InternalCloneRecord(const std::string& acRecordName, TweakDBID aClonedRecordDBID,
                                  std::shared_ptr<spdlog::logger> aLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::Handle<RED4ext::IScriptable> record;
    if (!pTDB->TryGetRecord(aClonedRecordDBID, record))
    {
        if (aLogger)
        {
            aLogger->info("Failed to create record '{}'. reason: Couldn't find record '{}' to clone", acRecordName,
                          CET::Get().GetVM().GetTDBIDString(aClonedRecordDBID));
        }
        return false;
    }

    auto* pTweakRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(record.GetPtr());
    return InternalCloneRecord(acRecordName, pTweakRecord, true, aLogger);
}

bool TweakDB::InternalCloneRecord(const std::string& acRecordName, const RED4ext::gamedataTweakDBRecord* acClonedRecord,
                                  bool cloneValues, std::shared_ptr<spdlog::logger> aLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    TweakDBID recordDBID(acRecordName);

    if (!pTDB->CreateRecord(recordDBID, acClonedRecord->GetTweakBaseHash()))
    {
        aLogger->info("Failed to create record '{}'. reason: Record already exists", acRecordName);
        return false;
    }

    auto& vm = CET::Get().GetVM();
    vm.RegisterTDBIDString(recordDBID, 0, acRecordName);

    // List of flats the game tried to read for our record
    TiltedPhoques::Vector<uint64_t> recordFlats;
    vm.GetTDBIDDerivedFrom(recordDBID, recordFlats);
    bool success = true;
    size_t lastCreatedFlatIdx;
    for (lastCreatedFlatIdx = 0; lastCreatedFlatIdx != recordFlats.size(); ++lastCreatedFlatIdx)
    {
        TweakDBID flatID = recordFlats[lastCreatedFlatIdx];
        const TDBIDLookupEntry lookup = vm.GetTDBIDLookupEntry(flatID);
        const std::string& propertyName = lookup.name;

        const auto* clonedFlatValue = pTDB->GetFlatValue(acClonedRecord->recordID + propertyName);
        if (clonedFlatValue == nullptr)
        {
            if (aLogger)
            {
                aLogger->info("Failed to create record '{}'. reason: Couldn't find flat '<...>{}'", acRecordName,
                              propertyName);
            }
            success = false;
            break;
        }        

        RED4ext::CStackType clonedStacktype = clonedFlatValue->GetValue();
        FlatPool* pPool = FlatPool::GetPool(clonedStacktype.type);
        if (pPool == nullptr)
        {
            // This should never happen
            assert(false);
            success = false;
            break;
        }

        if (cloneValues)
        {
            flatID.SetTDBOffset(pPool->Get(clonedStacktype));
        }
        else
        {
            // get default FlatValue for this flat type
            RED4ext::CName flatTypeName;
            clonedStacktype.type->GetName(flatTypeName);
            const auto* pDefaultFlatValue = pTDB->GetDefaultFlatValue(flatTypeName);
            RED4ext::CStackType defaultStackType = pDefaultFlatValue->GetValue();

            flatID.SetTDBOffset(pPool->GetOrCreate(defaultStackType, pDefaultFlatValue->ToTDBOffset()));
        }

        // add our flat pointing to that value
        if (!pTDB->AddFlat(flatID))
        {
            // Decreases 'm_useCount'
            pPool->Remove(flatID.ToTDBOffset());
            if (aLogger)
            {
                aLogger->info("Failed to create record '{}'. reason: Flat '{}' collision, pick another record name",
                              acRecordName, propertyName);
            }
            success = false;
            break;
        }
    }

    // revert changes
    if (!success)
    {
        vm.RemoveTDBIDDerivedFrom(recordDBID);
        // Only RemoveFlat the ones we made
        for (size_t i = 0; i != lastCreatedFlatIdx; ++i)
        {
            RemoveFlat(recordFlats[i]);
        }
        return false;
    }

    pTDB->UpdateRecord(recordDBID);

    std::lock_guard<std::mutex> _(s_mutex);
    s_createdRecords.emplace(recordDBID);
    return true;
}

bool TweakDB::InternalDeleteRecord(TweakDBID aDBID, std::shared_ptr<spdlog::logger> aLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    if (!IsACreatedRecord(aDBID))
    {
        if (aLogger)
        {
            aLogger->info("Record '{}' couldn't be deleted. reason: Record not found",
                          CET::Get().GetVM().GetTDBIDString(aDBID));
        }
        return false;
    }

    if (!pTDB->RemoveRecord(aDBID))
    {
        if (aLogger)
        {
            aLogger->info("Record '{}' couldn't be deleted. reason: Unknown", CET::Get().GetVM().GetTDBIDString(aDBID));
        }
        return false; // shouldn't happen
    }

    auto& vm = CET::Get().GetVM();
    TiltedPhoques::Vector<uint64_t> recordFlats;
    vm.GetTDBIDDerivedFrom(aDBID, recordFlats);
    for (const auto& flatID : recordFlats)
    {
        // This shouldn't delete any game-created flats
        RemoveFlat(flatID);
    }
    vm.RemoveTDBIDDerivedFrom(aDBID);

    std::lock_guard<std::mutex> _(s_mutex);
    s_createdRecords.erase(aDBID);
    return true;
}

bool TweakDB::RemoveFlat(TweakDBID aDBID)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pFlatValue = pTDB->GetFlatValue(aDBID);
    if (pFlatValue == nullptr)
        return false;

    FlatPool* pPool = FlatPool::GetPool(pFlatValue->GetValue().type);
    if (pPool == nullptr)
    {
        // This should never happen
        assert(false);
        return false;
    }

    pPool->Remove(pFlatValue->ToTDBOffset());
    return pTDB->RemoveFlat(aDBID);
}

bool TweakDB::IsACreatedRecord(TweakDBID aDBID)
{
    std::lock_guard<std::mutex> _(s_mutex);
    return s_createdRecords.find(aDBID) != s_createdRecords.end();
}

FlatPool::Item::Item(HashType aHash, int32_t aTDBOffset) noexcept
    : m_hash(aHash)
    , m_useCount(1)
    , m_tdbOffset(aTDBOffset)
{
}

void FlatPool::Item::DecUseCount()
{
    if (m_useCount == 0)
    {
        // This should never happen if the pool is used properly
        assert(false);
        return;
    }

    --m_useCount;
    if (m_useCount == 0)
    {
        m_availableAt = clock::now() + c_recycleWaitTime;
    }
}

void FlatPool::Item::IncUseCount()
{
    ++m_useCount;
}

bool FlatPool::Item::ReadyForRecycle()
{
    return m_useCount == 0 && clock::now() >= m_availableAt;
}

RED4ext::TweakDB::FlatValue* FlatPool::Item::ToFlatValue()
{
    auto* pTDB = RED4ext::TweakDB::Get();

    int32_t offset = m_tdbOffset & 0x00FFFFFF;
    if (offset == 0)
        return nullptr;

    return reinterpret_cast<RED4ext::TweakDB::FlatValue*>(pTDB->flatDataBuffer + offset);
}

int32_t FlatPool::Get(const RED4ext::CStackType& acStackType)
{
    return Get(acStackType, HashValue(acStackType));
}

int32_t FlatPool::Get(const RED4ext::CStackType& acStackType, HashType aHash)
{
    std::lock_guard<std::mutex> _(m_mutex);

    for (auto& item : m_items)
    {
        if (item.m_hash != aHash)
            continue;

        const auto* pFlatValue = item.ToFlatValue();
        RED4ext::CStackType poolStackType = pFlatValue->GetValue();

        assert(poolStackType.type == acStackType.type);

        if (poolStackType.type->IsEqual(poolStackType.value, acStackType.value))
        {
            item.IncUseCount();
            return item.m_tdbOffset;
        }
    }

    return -1;
}

int32_t FlatPool::Create(const RED4ext::CStackType& acStackType, int32_t aTDBOffset)
{
    return Create(acStackType, HashValue(acStackType), aTDBOffset);
}

int32_t FlatPool::Create(const RED4ext::CStackType& acStackType, HashType aHash, int32_t aTDBOffset)
{
    std::lock_guard<std::mutex> _(m_mutex);

    if (aTDBOffset == -1)
    {
        for (auto& item : m_items)
        {
            if (!item.ReadyForRecycle())
                continue;

            // Overwrite the flat
            if (item.ToFlatValue()->SetValue(acStackType))
            {
                item.m_hash = HashValue(acStackType);
                item.IncUseCount();
                return item.m_tdbOffset;
            }
        }

        int32_t tdbOffset = RED4ext::TweakDB::Get()->CreateFlatValue(acStackType);
        if (tdbOffset != -1)
        {
            m_items.emplace_back(aHash, tdbOffset);
        }
        return tdbOffset;
    }
    else
    {
        m_items.emplace_back(aHash, aTDBOffset);
        return aTDBOffset;
    }
}

int32_t FlatPool::GetOrCreate(const RED4ext::CStackType& acStackType, int32_t aTDBOffset)
{
    return GetOrCreate(acStackType, HashValue(acStackType), aTDBOffset);
}

int32_t FlatPool::GetOrCreate(const RED4ext::CStackType& acStackType, HashType aHash, int32_t aTDBOffset)
{
    int32_t existingTDBOffset = Get(acStackType, aHash);
    if (existingTDBOffset != -1)
    {
        assert((aTDBOffset == -1) || (existingTDBOffset == aTDBOffset));
        return existingTDBOffset;
    }

    return Create(acStackType, aHash, aTDBOffset);
}

bool FlatPool::Remove(int32_t aTDBOffset)
{
    std::lock_guard<std::mutex> _(m_mutex);

    for (Item& item : m_items)
    {
        if (item.m_tdbOffset != aTDBOffset)
            continue;

        item.DecUseCount();
        return true;
    }

    return false;
}

FlatPool::HashType FlatPool::HashValue(const RED4ext::CStackType& acStackType)
{
    return HashValue(acStackType, m_poolType);
}

FlatPool::HashType FlatPool::HashValue(const RED4ext::CStackType& acStackType, Type aType, HashType aSeed)
{
    switch (aType)
    {
    case Type::ArrayTweakDBID:
    case Type::ArrayQuaternion:
    case Type::ArrayEulerAngles:
    case Type::ArrayVector3:
    case Type::ArrayVector2:
    case Type::ArrayColor:
    case Type::ArrayGamedataLocKeyWrapper:
    case Type::ArrayRaRefCResource:
    case Type::ArrayCName:
    case Type::ArrayBool:
    case Type::ArrayString:
    case Type::ArrayFloat:
    case Type::ArrayInt32:
    {
        if (acStackType.type->GetType() != RED4ext::ERTTIType::Array)
        {
            RED4ext::CName typeName;
            acStackType.type->GetName(typeName);
            auto exceptionStr = fmt::format("Unexpected RTTI type: {}", typeName.ToString());
            // This should never happen
            throw std::exception(exceptionStr.c_str());
        }

        auto* pArrayType = reinterpret_cast<RED4ext::CArray*>(acStackType.type);
        auto* pArrayInnerType = pArrayType->GetInnerType();
        auto arrayInnerPoolType = RTTIToPoolType(pArrayInnerType);
        if (arrayInnerPoolType == Type::Unknown)
        {
            RED4ext::CName typeName;
            pArrayInnerType->GetName(typeName);
            auto exceptionStr = fmt::format("Unexpected Array inner RTTI type: {}", typeName.ToString());
            // This should never happen
            throw std::exception(exceptionStr.c_str());
        }

        HashType hash = aSeed;
        uint32_t arraySize = pArrayType->GetLength(acStackType.value);
        for (uint32_t i = 0; i != arraySize; ++i)
        {
            RED4ext::CStackType innerStackType;
            innerStackType.type = pArrayInnerType;
            innerStackType.value = pArrayType->GetElement(acStackType.value, i);
            hash = HashValue(innerStackType, arrayInnerPoolType, hash);
        }

        return hash;
    }

    case Type::Quaternion:
    case Type::EulerAngles:
    case Type::Vector3:
    case Type::Vector2:
    case Type::Color:
    case Type::GamedataLocKeyWrapper:
    case Type::RaRefCResource:
    case Type::CName:
    case Type::Bool:
    case Type::Float:
    case Type::Int32:
    {
        auto* pData = reinterpret_cast<uint8_t*>(acStackType.value);
        return HashFunc(pData, acStackType.type->GetSize(), aSeed);
    }

    case Type::TweakDBID:
    {
        auto* pData = reinterpret_cast<uint8_t*>(acStackType.value);
        return HashFunc(pData, 4 + 1 /* nameHash + nameLen */, aSeed);
    }

    case Type::String:
    {
        auto* pString = reinterpret_cast<RED4ext::CString*>(acStackType.value);
        auto* pData = reinterpret_cast<const uint8_t*>(pString->c_str());
        return HashFunc(pData, pString->Length(), aSeed);
    }
    default:
    {
        // This should never happen
        throw std::exception("Unknown PoolType");
    }
    }
}

bool FlatPool::Initialize()
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> _1(mutex);
    if (s_initialized)
        return true;

    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock<RED4ext::SharedMutex> _2(pTDB->mutex00);
    if (pTDB->flatDataBufferCapacity == 0)
        return false; // TweakDB is not initialized yet

    for (size_t i = 0; i != (size_t)FlatPool::Type::Count; ++i)
    {
        s_pools[i].m_poolType = (FlatPool::Type)i;
    }

    // key: tdbOffset
    // value: FlatPool::m_items index
    // reason: optimization, much faster than linear lookup
    std::map<int32_t, size_t> tdbOffToIndex;
    for (const TweakDBID flatID : pTDB->flats)
    {
        auto* pFlatValue = pTDB->GetFlatValue(flatID);
        RED4ext::CStackType stackType = pFlatValue->GetValue();
        FlatPool* pPool;
        //FlatPool* pPool = GetPool(stackType.type);
        {
            Type flatPoolType = RTTIToPoolType(stackType.type);
            if (flatPoolType == Type::Unknown)
            {
                // This should never happen
                assert(false);
                continue;
            }
            else
            {
                pPool = &s_pools[static_cast<size_t>(flatPoolType)];
            }
        }

        int32_t tdbOffset = pFlatValue->ToTDBOffset();
        const auto it = tdbOffToIndex.lower_bound(tdbOffset);
        if (it != tdbOffToIndex.end() && it->first == tdbOffset)
        {
            std::lock_guard<std::mutex> _3(pPool->m_mutex);
            pPool->m_items[it->second].IncUseCount(); 
        }
        else
        {
            tdbOffToIndex.emplace(tdbOffset, pPool->m_items.size());
            pPool->Create(stackType, tdbOffset);
        }
    }

    s_initialized = true;
    return true;
}

FlatPool* FlatPool::GetPool(const RED4ext::IRTTIType* acpType)
{
    if (!Initialize())
        return nullptr;

    Type flatPoolType = RTTIToPoolType(acpType);
    if (flatPoolType == Type::Unknown)
        return nullptr;

    return &s_pools[static_cast<size_t>(flatPoolType)];
}

FlatPool::Type FlatPool::RTTIToPoolType(const RED4ext::IRTTIType* acpType)
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

    FlatPool::Type poolType = FlatPool::Type::Unknown;
    if (acpType == pArrayTweakDBIDType)
        poolType = FlatPool::Type::ArrayTweakDBID;
    else if (acpType == pTweakDBIDType)
        poolType = FlatPool::Type::TweakDBID;
    else if (acpType == pArrayQuaternionType)
        poolType = FlatPool::Type::ArrayQuaternion;
    else if (acpType == pQuaternionType)
        poolType = FlatPool::Type::Quaternion;
    else if (acpType == pArrayEulerAnglesType)
        poolType = FlatPool::Type::ArrayEulerAngles;
    else if (acpType == pEulerAnglesType)
        poolType = FlatPool::Type::EulerAngles;
    else if (acpType == pArrayVector3Type)
        poolType = FlatPool::Type::ArrayVector3;
    else if (acpType == pVector3Type)
        poolType = FlatPool::Type::Vector3;
    else if (acpType == pArrayVector2Type)
        poolType = FlatPool::Type::ArrayVector2;
    else if (acpType == pVector2Type)
        poolType = FlatPool::Type::Vector2;
    else if (acpType == pArrayColorType)
        poolType = FlatPool::Type::ArrayColor;
    else if (acpType == pColorType)
        poolType = FlatPool::Type::Color;
    else if (acpType == pArrayGamedataLocKeyWrapperType)
        poolType = FlatPool::Type::ArrayGamedataLocKeyWrapper;
    else if (acpType == pGamedataLocKeyWrapperType)
        poolType = FlatPool::Type::GamedataLocKeyWrapper;
    else if (acpType == pArrayRaRefCResourceType)
        poolType = FlatPool::Type::ArrayRaRefCResource;
    else if (acpType == pRaRefCResourceType)
        poolType = FlatPool::Type::RaRefCResource;
    else if (acpType == pArrayCNameType)
        poolType = FlatPool::Type::ArrayCName;
    else if (acpType == pCNameType)
        poolType = FlatPool::Type::CName;
    else if (acpType == pArrayBoolType)
        poolType = FlatPool::Type::ArrayBool;
    else if (acpType == pBoolType)
        poolType = FlatPool::Type::Bool;
    else if (acpType == pArrayStringType)
        poolType = FlatPool::Type::ArrayString;
    else if (acpType == pStringType)
        poolType = FlatPool::Type::String;
    else if (acpType == pArrayFloatType)
        poolType = FlatPool::Type::ArrayFloat;
    else if (acpType == pFloatType)
        poolType = FlatPool::Type::Float;
    else if (acpType == pArrayInt32Type)
        poolType = FlatPool::Type::ArrayInt32;
    else if (acpType == pInt32Type)
        poolType = FlatPool::Type::Int32;

    return poolType;
}
