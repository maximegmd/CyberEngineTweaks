#pragma once

#include <reverse/BasicTypes.h>
#include <reverse/TweakDB/FlatPool.h>

namespace RED4ext
{
struct gamedataTweakDBRecord;
}

struct TweakDB
{
    TweakDB(const TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref& aLua);

    void DebugStats();
    sol::object GetRecords(const std::string& acRecordTypeName);
    sol::object GetRecordByName(const std::string& acRecordName);
    sol::object GetRecord(TweakDBID aDBID);
    sol::object QueryByName(const std::string& acQueryName);
    sol::object Query(TweakDBID aDBID);
    sol::object GetFlatByName(const std::string& acFlatName);
    sol::object GetFlat(TweakDBID aDBID);
    bool SetFlatsByName(const std::string& acRecordName, sol::table aTable, sol::this_environment aThisEnv);
    bool SetFlats(TweakDBID aDBID, sol::table aTable, sol::this_environment aThisEnv);
    bool SetFlatByName(const std::string& acFlatName, sol::object aObject, sol::this_environment aThisEnv);
    bool SetFlat(TweakDBID aDBID, sol::object aObject, sol::this_environment aThisEnv);
    bool SetFlatByNameAutoUpdate(const std::string& acFlatName, sol::object aObject, sol::this_environment aThisEnv);
    bool SetFlatAutoUpdate(TweakDBID aDBID, sol::object aObject, sol::this_environment aThisEnv);
    bool SetTypedFlatByName(const std::string& acFlatName, sol::object aObject, const std::string& acTypeName,
                            sol::this_environment aThisEnv);
    bool SetTypedFlat(TweakDBID aDBID, sol::object aObject, const std::string& acTypeName,
                      sol::this_environment aThisEnv);
    bool UpdateRecordByName(const std::string& acRecordName);
    bool UpdateRecordByID(TweakDBID aDBID);
    bool UpdateRecord(sol::object aValue, sol::this_environment aThisEnv);
    bool CreateRecord(const std::string& acRecordName, const std::string& acRecordTypeName,
                      sol::this_environment aThisEnv);
    bool CreateRecordToID(TweakDBID aDBID, const std::string& acRecordTypeName,
                      sol::this_environment aThisEnv);
    bool CloneRecordByName(const std::string& acRecordName, const std::string& acClonedRecordName,
                           sol::this_environment aThisEnv);
    bool CloneRecord(const std::string& acRecordName, TweakDBID aClonedRecordDBID, sol::this_environment aThisEnv);
    bool CloneRecordToID(TweakDBID aDBID, TweakDBID aClonedRecordDBID, sol::this_environment aThisEnv);
    bool DeleteRecord(const std::string& acRecordName, sol::this_environment aThisEnv);
    bool DeleteRecordByID(TweakDBID aDBID, sol::this_environment aThisEnv);

protected:
    friend struct TweakDBEditor;
    bool SetOrCreateFlat(TweakDBID aDBID, sol::object aObject, const std::string& acFlatName,
                         const std::string& acTypeName, std::shared_ptr<spdlog::logger> aLogger = nullptr);
    static RED4ext::CStackType InternalGetFlat(RED4ext::TweakDBID aDBID);
    static int32_t InternalSetFlat(RED4ext::TweakDBID aDBID, const RED4ext::CStackType& acStackType);
    static bool InternalCreateRecord(const std::string& acRecordName, const std::string& acRecordTypeName,
                                     const std::shared_ptr<spdlog::logger>& aLogger);
    static bool InternalCreateRecord(TweakDBID aDBID, const std::string& acRecordTypeName,
                                     const std::shared_ptr<spdlog::logger>& aLogger);
    static bool InternalCloneRecord(const std::string& acRecordName, RED4ext::TweakDBID aClonedRecordDBID,
                                    const std::shared_ptr<spdlog::logger>& aLogger);
    static bool InternalCloneRecord(TweakDBID aDBID, RED4ext::TweakDBID aClonedRecordDBID,
                                    const std::shared_ptr<spdlog::logger>& aLogger);
    // Can't figure out a good name for this function.
    // Creates a record of the same type as 'acClonedRecord'
    // Creates all of its flats
    // Setting 'cloneValues' to false will set default values
    static bool InternalCloneRecord(const std::string& acRecordName,
                                    const RED4ext::gamedataTweakDBRecord* acClonedRecord, bool cloneValues,
                                    const std::shared_ptr<spdlog::logger>& aLogger);
    static bool InternalCloneRecord(TweakDBID aDBID,
                                    const RED4ext::gamedataTweakDBRecord* acClonedRecord, bool cloneValues,
                                    const std::shared_ptr<spdlog::logger>& aLogger);
    static bool InternalCloneFlats(RED4ext::TweakDBID aDBID,
                                   const RED4ext::gamedataTweakDBRecord* acClonedRecord, bool cloneValues,
                                   const std::shared_ptr<spdlog::logger>& aLogger);
    static bool InternalDeleteRecord(RED4ext::TweakDBID aDBID, std::shared_ptr<spdlog::logger> aLogger = nullptr);
    static bool RemoveFlat(RED4ext::TweakDBID aDBID);
    static bool IsACreatedRecord(RED4ext::TweakDBID aDBID);
    inline static std::string GetTDBIDString(uint64_t aDBID);

private: 
    TiltedPhoques::Lockable<sol::state, std::recursive_mutex>::Ref m_lua;
    static TiltedPhoques::UniquePtr<FlatPool> s_flatPool;
    static std::mutex s_mutex;
    static std::set<RED4ext::TweakDBID> s_createdRecords;
};