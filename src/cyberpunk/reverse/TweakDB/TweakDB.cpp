#include <stdafx.h>

#include "TweakDB.h"

#include "EngineTweaks.h"
#include <reverse/WeakReference.h>
#include <reverse/StrongReference.h>
#include <reverse/RTTIMapper.h>
#include <scripting/Scripting.h>

using namespace std::chrono_literals;

std::mutex TweakDB::s_mutex;
std::set<RED4ext::TweakDBID> TweakDB::s_createdRecords;
TiltedPhoques::UniquePtr<FlatPool> TweakDB::s_flatPool;

TweakDB::TweakDB(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aLua)
    : m_lua(aLua)
{
    if (!s_flatPool)
        s_flatPool = TiltedPhoques::MakeUnique<FlatPool>();
}

void TweakDB::DebugStats()
{
    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock _1(pTDB->mutex00);
    std::shared_lock _2(pTDB->mutex01);
    const auto logger = spdlog::get("scripting"); // DebugStats should always log to console

    logger->info("flats: {}", pTDB->flats.size);
    logger->info("records: {}", pTDB->recordsByID.size);
    logger->info("queries: {}", pTDB->queries.GetSize());
    size_t flatDataBufferSize = pTDB->flatDataBufferEnd - pTDB->flatDataBuffer;
    logger->info("flatDataBuffer: {}/{} bytes", flatDataBufferSize, pTDB->flatDataBufferCapacity);
    logger->info("created records: {}", s_createdRecords.size());
}

sol::object TweakDB::GetRecords(const std::string& acRecordTypeName) const
{
    static auto* pArrayType = RED4ext::CRTTISystem::Get()->GetType("array:handle:IScriptable");
    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock _(pTDB->mutex01);

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

sol::object TweakDB::GetRecordByName(const std::string& acRecordName) const
{
    return GetRecord(TweakDBID(acRecordName));
}

sol::object TweakDB::GetRecord(TweakDBID aDBID) const
{
    auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::Handle<RED4ext::IScriptable> record;
    if (!pTDB->TryGetRecord(aDBID.value, record))
        return sol::nil;

    auto state = m_lua.Lock();
    return make_object(state.Get(), StrongReference(m_lua, std::move(record)));
}

sol::object TweakDB::QueryByName(const std::string& acQueryName) const
{
    return Query(TweakDBID(acQueryName));
}

sol::object TweakDB::Query(TweakDBID aDBID) const
{
    static auto* pArrayTweakDBIDType = RED4ext::CRTTISystem::Get()->GetType("array:TweakDBID");
    auto* pTDB = RED4ext::TweakDB::Get();
    std::shared_lock _(pTDB->mutex01);

    RED4ext::DynArray<RED4ext::TweakDBID> queryResult;
    if (!pTDB->TryQuery(aDBID.value, queryResult))
        return sol::nil;

    RED4ext::CStackType stackType(pArrayTweakDBIDType, &queryResult);
    auto state = m_lua.Lock();
    return Scripting::ToLua(state, stackType);
}

sol::object TweakDB::GetFlatByName(const std::string& acFlatName) const
{
    return GetFlat(TweakDBID(acFlatName));
}

sol::object TweakDB::GetFlat(TweakDBID aDBID) const
{
    RED4ext::CStackType data = InternalGetFlat(aDBID.value);

    if (!data.value)
        return sol::nil;

    auto state = m_lua.Lock();
    return Scripting::ToLua(state, data);
}

bool TweakDB::SetFlatsByName(const std::string& acRecordName, sol::table aTable, sol::this_environment aThisEnv)
{
    return SetFlats(TweakDBID(acRecordName), std::move(aTable), std::move(aThisEnv));
}

bool TweakDB::SetFlats(TweakDBID aDBID, sol::table aTable, sol::this_environment aThisEnv)
{
    bool success = true;
    const TweakDBID prepDBID(aDBID, ".");

    for (auto& [key, value] : aTable)
    {
        if (!key.is<std::string>())
            continue;

        const TweakDBID flatDBID(prepDBID, key.as<std::string>());
        success &= SetFlat(flatDBID, value, aThisEnv);
    }

    UpdateRecordByID(aDBID);

    return success;
}

bool TweakDB::SetFlatByName(const std::string& acFlatName, sol::object aObject, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return SetOrCreateFlat(TweakDBID(acFlatName), std::move(aObject), acFlatName, "", logger);
}

bool TweakDB::SetFlat(TweakDBID aDBID, sol::object aObject, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return SetOrCreateFlat(aDBID, std::move(aObject), "", "", logger);
}

bool TweakDB::SetFlatByNameAutoUpdate(const std::string& acFlatName, sol::object aObject, sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    const TweakDBID dbid(acFlatName);
    if (SetOrCreateFlat(dbid, std::move(aObject), acFlatName, "", logger))
    {
        const uint64_t recordDBID = EngineTweaks::Get().GetVM().GetTDBIDBase(dbid.value);
        if (recordDBID != 0)
            UpdateRecordByID(recordDBID);

        return true;
    }

    return false;
}

bool TweakDB::SetFlatAutoUpdate(TweakDBID aDBID, sol::object aObject, sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    if (SetOrCreateFlat(aDBID, std::move(aObject), "", "", logger))
    {
        const uint64_t recordDBID = EngineTweaks::Get().GetVM().GetTDBIDBase(aDBID.value);
        if (recordDBID != 0)
            UpdateRecordByID(recordDBID);

        return true;
    }

    return false;
}

bool TweakDB::SetTypedFlatByName(const std::string& acFlatName, sol::object aObject, const std::string& acTypeName, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return SetOrCreateFlat(TweakDBID(acFlatName), aObject, acFlatName, acTypeName, logger);
}

bool TweakDB::SetTypedFlat(TweakDBID aDBID, sol::object aObject, const std::string& acTypeName, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return SetOrCreateFlat(aDBID, aObject, "", acTypeName, logger);
}

bool TweakDB::SetOrCreateFlat(
    TweakDBID aDBID, sol::object aObject, const std::string& acFlatName, const std::string& acTypeName, const std::shared_ptr<spdlog::logger>& aLogger) const
{
    auto* pTDB = RED4ext::TweakDB::Get();
    static thread_local TiltedPhoques::ScratchAllocator s_scratchMemory(1 << 22);
    struct ResetAllocator
    {
        ~ResetAllocator() { s_scratchMemory.Reset(); }
    };
    ResetAllocator ___allocatorReset;

    RED4ext::TweakDBID* pFlat;
    RED4ext::CStackType data;

    {
        std::shared_lock _(pTDB->mutex00);
        pFlat = pTDB->flats.Find(aDBID.value);

        if (pFlat != pTDB->flats.End())
            data = s_flatPool->GetData(pFlat->ToTDBOffset());
    }

    if (data.value)
    {
        data = Scripting::ToRED(aObject, data.type, &s_scratchMemory);
        if (data.value == nullptr)
        {
            if (aLogger)
            {
                const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
                aLogger->info("[TweakDB::SetFlat] Failed to convert value for {}. Expecting: {}", flatName, data.type->GetName().ToString());
            }

            return false;
        }

        const int32_t newTDBOffset = s_flatPool->AllocateData(data);
        if (newTDBOffset == FlatPool::InvalidOffset)
        {
            if (aLogger)
            {
                const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
                aLogger->info("[TweakDB::SetFlat] Failed to allocate flat value for {}", flatName);
            }

            return false;
        }

        {
            std::shared_lock _(pTDB->mutex00);
            pFlat = pTDB->flats.Find(aDBID.value);

            if (pFlat == pTDB->flats.End())
            {
                if (aLogger)
                {
                    const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
                    aLogger->info("[TweakDB::SetFlat] Failed to update an existing flat {}", flatName);
                }

                return false;
            }

            pFlat->SetTDBOffset(newTDBOffset);
        }

        return true;
    }

    auto* pRTTI = RED4ext::CRTTISystem::Get();

    if (!acTypeName.empty())
        data.type = pRTTI->GetType(acTypeName.c_str());
    else
    {
        const auto cTypeName = RTTIMapper::TryResolveTypeName(aObject);

        if (cTypeName.IsNone())
        {
            if (aLogger)
            {
                const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
                aLogger->info("[TweakDB::SetFlat] Type for {} is ambiguous, use third parameter to specify the type", flatName);
            }

            return false;
        }

        data.type = pRTTI->GetType(cTypeName);
    }

    if (data.type == nullptr)
    {
        if (aLogger)
            aLogger->info("[TweakDB::SetFlat] Unknown type");

        return false;
    }

    if (!s_flatPool->IsFlatType(data.type))
    {
        if (aLogger)
            aLogger->info("[TweakDB::SetFlat] Unsupported type: {}", data.type->GetName().ToString());
        return false;
    }

    data = Scripting::ToRED(aObject, data.type, &s_scratchMemory);
    if (data.value == nullptr)
    {
        if (aLogger)
        {
            const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
            aLogger->info("[TweakDB::SetFlat] Failed to convert value for {}. Expecting: {}", flatName, data.type->GetName().ToString());
        }
        return false;
    }

    const int32_t newTDBOffset = s_flatPool->AllocateData(data);
    if (newTDBOffset == -1)
    {
        if (aLogger)
        {
            const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
            aLogger->info("[TweakDB::SetFlat] Failed to allocate flat value for {}", flatName);
        }
        return false;
    }

    RED4ext::TweakDBID flatID(aDBID.value);
    flatID.SetTDBOffset(newTDBOffset);

    if (!pTDB->AddFlat(flatID))
    {
        if (aLogger)
        {
            const std::string& flatName = !acFlatName.empty() ? acFlatName : GetTDBIDString(aDBID.value);
            aLogger->info("[TweakDB::SetFlat] Failed to create a new flat {}", flatName);
        }
        return false;
    }

    if (!acFlatName.empty())
        EngineTweaks::Get().GetVM().RegisterTDBIDString(aDBID.value, 0, acFlatName);

    return true;
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
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    RED4ext::gamedataTweakDBRecord* pRecord;
    if (aValue.is<StrongReference>())
        pRecord = static_cast<RED4ext::gamedataTweakDBRecord*>(aValue.as<StrongReference*>()->GetHandle());
    else if (aValue.is<WeakReference>())
        pRecord = static_cast<RED4ext::gamedataTweakDBRecord*>(aValue.as<WeakReference*>()->GetHandle());
    else
    {
        logger->info("[TweakDB::UpdateRecord] Expecting handle or whandle");
        return false;
    }

    return pTDB->UpdateRecord(pRecord);
}

bool TweakDB::CreateRecord(const std::string& acRecordName, const std::string& acRecordTypeName, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalCreateRecord(acRecordName, acRecordTypeName, logger);
}

bool TweakDB::CreateRecordToID(TweakDBID aDBID, const std::string& acRecordTypeName, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalCreateRecord(aDBID, acRecordTypeName, logger);
}

bool TweakDB::CloneRecordByName(const std::string& acRecordName, const std::string& acClonedRecordName, sol::this_environment aThisEnv) const
{
    return CloneRecord(acRecordName, TweakDBID(acClonedRecordName), std::move(aThisEnv));
}

bool TweakDB::CloneRecord(const std::string& acRecordName, TweakDBID aClonedRecordDBID, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalCloneRecord(acRecordName, aClonedRecordDBID.value, logger);
}

bool TweakDB::CloneRecordToID(TweakDBID aDBID, TweakDBID aClonedRecordDBID, sol::this_environment aThisEnv) const
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalCloneRecord(aDBID, aClonedRecordDBID.value, logger);
}

bool TweakDB::DeleteRecord(const std::string& acRecordName, sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalDeleteRecord(RED4ext::TweakDBID(acRecordName), logger);
}

bool TweakDB::DeleteRecordByID(TweakDBID aDBID, sol::this_environment aThisEnv)
{
    const sol::environment cEnv = aThisEnv;
    const auto logger = cEnv["__logger"].get<std::shared_ptr<spdlog::logger>>();

    return InternalDeleteRecord(RED4ext::TweakDBID(aDBID.name_hash, aDBID.name_length), logger);
}

RED4ext::CStackType TweakDB::InternalGetFlat(RED4ext::TweakDBID aDBID)
{
    if (!aDBID.IsValid())
        return {};

    auto* pTDB = RED4ext::TweakDB::Get();

    std::shared_lock _(pTDB->mutex00);
    const auto* flat = pTDB->flats.Find(aDBID);

    if (flat == pTDB->flats.End())
        return {};

    return s_flatPool->GetData(flat->ToTDBOffset());
}

int32_t TweakDB::InternalSetFlat(RED4ext::TweakDBID aDBID, const RED4ext::CStackType& acStackType)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    const int32_t newTDBOffset = s_flatPool->AllocateData(acStackType);
    if (newTDBOffset == FlatPool::InvalidOffset)
        return newTDBOffset;

    {
        std::shared_lock _(pTDB->mutex00);

        auto* pDBID = pTDB->flats.Find(aDBID);
        if (pDBID == pTDB->flats.End())
            return FlatPool::InvalidOffset;

        pDBID->SetTDBOffset(newTDBOffset);
    }

    return newTDBOffset;
}

bool TweakDB::InternalCreateRecord(const std::string& acRecordName, const std::string& acRecordTypeName, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pType = RED4ext::CRTTISystem::Get()->GetType(acRecordTypeName.c_str());
    RED4ext::Handle<RED4ext::IScriptable> record;
    {
        std::shared_lock _(pTDB->mutex01);

        RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> recordsOfSameType;
        if (!pTDB->TryGetRecordsByType(pType, recordsOfSameType) || recordsOfSameType.size == 0)
        {
            if (acpLogger)
                acpLogger->info("Failed to create record '{}'. reason: Unknown type '{}'", acRecordName, acRecordTypeName);

            return false;
        }
        record = recordsOfSameType[0];
    }

    const auto* pTweakRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(record.GetPtr());
    return InternalCloneRecord(acRecordName, pTweakRecord, false, acpLogger);
}

bool TweakDB::InternalCreateRecord(TweakDBID aDBID, const std::string& acRecordTypeName, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pType = RED4ext::CRTTISystem::Get()->GetType(acRecordTypeName.c_str());
    RED4ext::Handle<RED4ext::IScriptable> record;
    {
        std::shared_lock _(pTDB->mutex01);

        RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> recordsOfSameType;
        if (!pTDB->TryGetRecordsByType(pType, recordsOfSameType) || recordsOfSameType.size == 0)
        {
            if (acpLogger)
                acpLogger->info("Failed to create record '{}'. reason: Unknown type '{}'", aDBID.ToString(), acRecordTypeName);

            return false;
        }
        record = recordsOfSameType[0];
    }

    const auto* pTweakRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(record.GetPtr());
    return InternalCloneRecord(aDBID, pTweakRecord, false, acpLogger);
}

bool TweakDB::InternalCloneRecord(const std::string& acRecordName, RED4ext::TweakDBID aClonedRecordDBID, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::Handle<RED4ext::IScriptable> record;
    if (!pTDB->TryGetRecord(aClonedRecordDBID, record))
    {
        if (acpLogger)
            acpLogger->info("Failed to create record '{}'. reason: Couldn't find record '{}' to clone", acRecordName, GetTDBIDString(aClonedRecordDBID));

        return false;
    }

    const auto* pTweakRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(record.GetPtr());
    return InternalCloneRecord(acRecordName, pTweakRecord, true, acpLogger);
}

bool TweakDB::InternalCloneRecord(TweakDBID aDBID, RED4ext::TweakDBID aClonedRecordDBID, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    RED4ext::Handle<RED4ext::IScriptable> record;
    if (!pTDB->TryGetRecord(aClonedRecordDBID, record))
    {
        if (acpLogger)
            acpLogger->info("Failed to create record {}. reason: Couldn't find record '{}' to clone", aDBID.ToString(), GetTDBIDString(aClonedRecordDBID));

        return false;
    }

    const auto* pTweakRecord = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(record.GetPtr());
    return InternalCloneRecord(aDBID, pTweakRecord, true, acpLogger);
}

bool TweakDB::InternalCloneRecord(
    const std::string& acRecordName, const RED4ext::gamedataTweakDBRecord* acClonedRecord, bool cloneValues, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    const RED4ext::TweakDBID recordDBID(acRecordName);

    if (!pTDB->CreateRecord(recordDBID, acClonedRecord->GetTweakBaseHash()))
    {
        if (acpLogger)
            acpLogger->info("Failed to create record '{}'. reason: Record already exists", acRecordName);

        return false;
    }

    auto& vm = EngineTweaks::Get().GetVM();
    vm.RegisterTDBIDString(recordDBID, 0, acRecordName);

    return InternalCloneFlats(recordDBID, acClonedRecord, cloneValues, acpLogger);
}

bool TweakDB::InternalCloneRecord(TweakDBID aDBID, const RED4ext::gamedataTweakDBRecord* acClonedRecord, bool cloneValues, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    const RED4ext::TweakDBID recordDBID(aDBID.name_hash, aDBID.name_length);

    if (!pTDB->CreateRecord(recordDBID, acClonedRecord->GetTweakBaseHash()))
    {
        if (acpLogger)
            acpLogger->info("Failed to create record {}. reason: Record already exists", aDBID.ToString());

        return false;
    }

    return InternalCloneFlats(recordDBID, acClonedRecord, cloneValues, acpLogger);
}

bool TweakDB::InternalCloneFlats(RED4ext::TweakDBID aDBID, const RED4ext::gamedataTweakDBRecord* acClonedRecord, bool cloneValues, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    auto& vm = EngineTweaks::Get().GetVM();

    // List of flats the game tried to read for our record
    TiltedPhoques::Vector<uint64_t> recordFlatIDs;
    vm.GetTDBIDDerivedFrom(aDBID, recordFlatIDs);
    bool success = true;
    size_t lastCreatedFlatIdx;

    RED4ext::SortedUniqueArray<RED4ext::TweakDBID> recordFlats;
    recordFlats.Reserve(static_cast<uint32_t>(recordFlatIDs.size()));

    for (lastCreatedFlatIdx = 0; lastCreatedFlatIdx != recordFlatIDs.size(); ++lastCreatedFlatIdx)
    {
        RED4ext::TweakDBID flatID = recordFlatIDs[lastCreatedFlatIdx];
        const std::string& propertyName = vm.GetTDBIDLookupEntry(flatID).name;

        RED4ext::CStackType data = InternalGetFlat(acClonedRecord->recordID + propertyName);

        if (!data.value)
        {
            if (acpLogger)
                acpLogger->info("Failed to create record {}. reason: Couldn't find flat '<...>{}'", TweakDBID(aDBID.value).ToString(), propertyName);

            success = false;
            break;
        }

        if (cloneValues)
            flatID.SetTDBOffset(s_flatPool->AllocateData(data));
        else
            flatID.SetTDBOffset(s_flatPool->AllocateDefault(data.type));

        recordFlats.Insert(flatID);
    }

    {
        std::lock_guard _(pTDB->mutex00);
        pTDB->flats.InsertOrAssign(recordFlats);
    }

    // revert changes
    if (!success)
    {
        vm.RemoveTDBIDDerivedFrom(aDBID);
        // Only RemoveFlat the ones we made
        for (size_t i = 0; i != lastCreatedFlatIdx; ++i)
            RemoveFlat(recordFlatIDs[i]);

        return false;
    }

    pTDB->UpdateRecord(aDBID);

    std::lock_guard _(s_mutex);
    s_createdRecords.emplace(aDBID);

    return true;
}

bool TweakDB::InternalDeleteRecord(RED4ext::TweakDBID aDBID, const std::shared_ptr<spdlog::logger>& acpLogger)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    if (!IsACreatedRecord(aDBID))
    {
        if (acpLogger)
            acpLogger->info("Record '{}' couldn't be deleted. reason: Record not found", GetTDBIDString(aDBID));

        return false;
    }

    if (!pTDB->RemoveRecord(aDBID))
    {
        if (acpLogger)
            acpLogger->info("Record '{}' couldn't be deleted. reason: Unknown", GetTDBIDString(aDBID));

        return false; // shouldn't happen
    }

    auto& vm = EngineTweaks::Get().GetVM();
    TiltedPhoques::Vector<uint64_t> recordFlats;
    vm.GetTDBIDDerivedFrom(aDBID, recordFlats);
    for (const auto& flatID : recordFlats)
    {
        // This shouldn't delete any game-created flats
        RemoveFlat(flatID);
    }
    vm.RemoveTDBIDDerivedFrom(aDBID);

    std::lock_guard _(s_mutex);
    s_createdRecords.erase(aDBID);
    return true;
}

bool TweakDB::RemoveFlat(RED4ext::TweakDBID aDBID)
{
    auto* pTDB = RED4ext::TweakDB::Get();
    return pTDB->RemoveFlat(aDBID);
}

bool TweakDB::IsACreatedRecord(RED4ext::TweakDBID aDBID)
{
    std::lock_guard _(s_mutex);
    return s_createdRecords.contains(aDBID);
}

std::string TweakDB::GetTDBIDString(uint64_t aDBID)
{
    return EngineTweaks::Get().GetVM().GetTDBIDString(aDBID);
}
