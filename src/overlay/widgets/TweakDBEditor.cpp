#include <stdafx.h>
#include <regex>
#include <shared_mutex>

#include <RED4ext/Types/TweakDB.hpp>
#include <RED4ext/Types/generated/Color.hpp>
#include <RED4ext/Types/generated/EulerAngles.hpp>
#include <RED4ext/Types/generated/Quaternion.hpp>
#include <RED4ext/Types/generated/Vector2.hpp>
#include <RED4ext/Types/generated/Vector3.hpp>

#include "HelperWidgets.h"
#include <CET.h>
#include <reverse/TweakDB.h>

#include "TweakDBEditor.h"

bool TweakDBEditor::s_recordsFilterIsRegex = false;
bool TweakDBEditor::s_flatsFilterIsRegex = false;
char TweakDBEditor::s_recordsFilterBuffer[256]{};
char TweakDBEditor::s_flatsFilterBuffer[256]{};
char TweakDBEditor::s_tweakdbidFilterBuffer[256]{};
float g_comboDropdownHeight = 300.0f;

struct CDPRTweakDBMetadata
{
    static constexpr char c_defaultFilename[] = "tweakdb.str";

    static CDPRTweakDBMetadata* Get()
    {
        static CDPRTweakDBMetadata instance;
        return &instance;
    }

    bool Initialize()
    {
        Reset();

        auto filepath = CET::Get().GetPaths().CETRoot() / c_defaultFilename;
        if (!std::filesystem::exists(filepath))
            return false;

        try
        {
            std::ifstream file(filepath, std::ios::binary);
            file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
            file.read(reinterpret_cast<char*>(&m_header), sizeof(Header));
            assert(m_header.m_version == 1);
            ReadTDBIDNameArray(file, m_header.m_recordsCount, m_records);
            ReadTDBIDNameArray(file, m_header.m_flatsCount, m_flats);
            ReadTDBIDNameArray(file, m_header.m_queriesCount, m_queries);
            m_isInitialized = true;
            return true;
        }
        // this is easier for now
        catch (std::exception&)
        {
            return false;
        }
    }

    bool IsInitialized()
    {
        return m_isInitialized;
    }

    bool GetRecordName(RED4ext::TweakDBID aDBID, std::string& aName)
    {
        const auto it = m_records.find(aDBID.value & 0xFFFFFFFFFF);
        if (it == m_records.end())
            return false;

        aName = it->second;
        return true;
    }

    bool GetFlatName(RED4ext::TweakDBID aDBID, std::string& aName)
    {
        const auto it = m_flats.find(aDBID.value & 0xFFFFFFFFFF);
        if (it == m_flats.end())
            return false;

        aName = it->second;
        return true;
    }

    bool GetQueryName(RED4ext::TweakDBID aDBID, std::string& aName)
    {
        const auto it = m_queries.find(aDBID.value & 0xFFFFFFFFFF);
        if (it == m_queries.end())
            return false;

        aName = it->second;
        return true;
    }

protected:
    int32_t ReadCompressedInt(std::ifstream& aFile)
    {
        int32_t uncompressed = 0;

        int8_t byte;
        aFile.read(reinterpret_cast<char*>(&byte), 1);
        uncompressed |= byte & 0x3F;
        if (byte & 0x40)
        {
            aFile.read(reinterpret_cast<char*>(&byte), 1);
            uncompressed |= (byte & 0x7F) << 6;
            if (byte & 0x80)
            {
                aFile.read(reinterpret_cast<char*>(&byte), 1);
                uncompressed |= (byte & 0x7F) << 13;
                if (byte & 0x80)
                {
                    aFile.read(reinterpret_cast<char*>(&byte), 1);
                    uncompressed |= (byte & 0x7F) << 20;
                    if (byte & 0x80)
                    {
                        aFile.read(reinterpret_cast<char*>(&byte), 1);
                        uncompressed |= byte << 27;
                    }
                }
            }
        }

        return uncompressed;
    }

    void ReadTDBIDNameArray(std::ifstream& aFile, uint32_t aCount, std::unordered_map<uint64_t, std::string>& aOutMap)
    {
        for (int32_t i = 0; i != aCount; ++i)
        {
            int32_t length = ReadCompressedInt(aFile);
            std::string str;
            str.resize(length);
            aFile.read(str.data(), length);
            aOutMap.try_emplace(TweakDBID(str).value, std::move(str));
        }
    }

    void Reset()
    {
        m_isInitialized = false;
        m_records.clear();
        m_flats.clear();
        m_queries.clear();
    }

private:
    struct Header
    {
        uint32_t m_magic;   // a hash of all types currently supported
        uint32_t m_version; // 1
        uint32_t m_recordsCount;
        uint32_t m_flatsCount;
        uint32_t m_queriesCount;
    };

    bool m_isInitialized = false;
    Header m_header;
    std::unordered_map<uint64_t, std::string> m_records;
    std::unordered_map<uint64_t, std::string> m_flats;
    std::unordered_map<uint64_t, std::string> m_queries;
};

namespace ImGui
{
std::pair<bool, std::string*> InputTextCStr(const char* acpLabel, const char* acpBuf, size_t aBufSize,
                                            ImGuiInputTextFlags aFlags = 0)
{
    if ((aFlags & ImGuiInputTextFlags_ReadOnly) == 0)
    {
        static bool isModified;
        static std::string tempStr;

        isModified = false;
        tempStr.clear();
        aFlags |= ImGuiInputTextFlags_CallbackResize;
        bool ret = ImGui::InputText(acpLabel, const_cast<char*>(acpBuf), aBufSize + 1, aFlags,
                                    [](ImGuiInputTextCallbackData* apData)
                                    {
                                        if (apData->EventFlag == ImGuiInputTextFlags_CallbackResize)
                                        {
                                            tempStr.resize(apData->BufTextLen);
                                            apData->Buf = tempStr.data();
                                            isModified = true;
                                        }
                                        return 0;
                                    });
        return {ret && isModified, &tempStr};
    }
    else
    {
        ImGui::InputText(acpLabel, const_cast<char*>(acpBuf), aBufSize, aFlags);
        return {false, nullptr};
    }
}

bool NextItemVisible(const ImVec2& aSize = ImVec2(1, 1), bool aClaimSpaceIfInvisible = true)
{
    ImVec2 rectMin = ImGui::GetCursorScreenPos();
    ImVec2 rectMax = ImVec2(rectMin.x + aSize.x, rectMin.y + aSize.y);
    bool visible = ImGui::IsRectVisible(rectMin, rectMax);
    if (!visible && aClaimSpaceIfInvisible)
    {
        ImGui::Dummy(aSize);
    }
    return visible;
}
}

bool SortTweakDBIDName(const std::string& acLeft, const std::string& acRight)
{
    // unknown TweakDBID should be at the bottom
    if (acLeft[0] == '<' && acRight[0] != '<')
        return false;
    else if (acLeft[0] != '<' && acRight[0] == '<')
        return true;

    size_t minLength = std::min(acLeft.size(), acRight.size());
    for (size_t i = 0; i != minLength; ++i)
    {
        char a = std::tolower(acLeft[i]);
        char b = std::tolower(acRight[i]);
        if (a != b)
            return a < b;
    }
    return acLeft.size() < acRight.size();
}

bool StringContains(const std::string_view& acString, const std::string_view& acSearch, bool aRegex = false)
{
    if (acSearch.size() == 0)
        return false;

    if (aRegex)
    {
        try
        {
            std::regex searchRegex(acSearch.begin(), acSearch.end());
            return std::regex_search(acString.begin(), acString.end(), searchRegex);
        }
        catch (std::regex_error&)
        {
            return false;
        }
    }
    else
    {
        const auto it = std::search(acString.begin(), acString.end(), acSearch.begin(), acSearch.end(),
                                    [](char a, char b) { return std::tolower(a) == std::tolower(b); });
        return it != acString.end();
    }
}

TweakDBEditor::TweakDBEditor(LuaVM& aVm)
    : m_vm(aVm)
{
}

void TweakDBEditor::OnEnable()
{
}

void TweakDBEditor::OnDisable()
{
}

void TweakDBEditor::Update()
{
    // LuaVM is initialized after TweakDB, let's wait for it
    if (!m_vm.IsInitialized())
    {
        ImGui::Text("TweakDB is not initialized yet");
        return;
    }

    if (!m_initialized)
    {
        CDPRTweakDBMetadata::Get()->Initialize();
        RefreshAll();
        m_initialized = true;
    }

    if (ImGui::BeginTabBar("TweakDBEditor-Bar"))
    {
        if (ImGui::BeginTabItem("Records"))
        {
            ImGui::BeginChild("Records");
            DrawRecordsTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Queries"))
        {
            ImGui::BeginChild("Queries");
            DrawQueriesTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Flats"))
        {
            ImGui::BeginChild("Flats");
            DrawFlatsTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Advanced"))
        {
            ImGui::BeginChild("Advanced");
            DrawAdvancedTab();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void TweakDBEditor::RefreshAll()
{
    RefreshRecords();
    RefreshFlats();
}

void TweakDBEditor::RefreshRecords()
{
    auto* pTDB = RED4ext::TweakDB::Get();

    std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex01);
    std::unordered_map<uint64_t, std::vector<CachedRecord>> map;

    size_t recordsCount = 0;
    pTDB->recordsByType.for_each(
        [this, &map, &recordsCount](RED4ext::IRTTIType* aRTTIType,
                                    RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> aRecords)
        {
            recordsCount += aRecords.size;
            RED4ext::CName typeName;
            aRTTIType->GetName(typeName);

            std::vector<CachedRecord>* recordsVec = &map[typeName];
            for (RED4ext::Handle<RED4ext::IScriptable> handle : aRecords)
            {
                auto* record = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(handle.GetPtr());
                std::string recordName = GetTweakDBIDStringRecord(record->recordID);
                if (TweakDB::IsACreatedRecord(record->recordID))
                    recordName.insert(0, "* ");
                CachedRecord cachedRecord(std::move(recordName), record->recordID);
                recordsVec->emplace_back(std::move(cachedRecord));
            }
        });

    m_cachedRecords.clear();
    m_cachedRecords.reserve(recordsCount);
    for (auto& [typeName, records] : map)
    {
        // order records inside group
        std::sort(records.begin(), records.end(), [](const CachedRecord& aLeft, const CachedRecord& aRight)
        {
            return SortTweakDBIDName(aLeft.m_name, aRight.m_name);
        });
        CachedRecordGroup recordGroup(typeName);
        recordGroup.m_records = std::move(records);
        m_cachedRecords.emplace_back(std::move(recordGroup));
    }

    // order groups
    std::sort(m_cachedRecords.begin(), m_cachedRecords.end(),
              [](const CachedRecordGroup& aLeft, const CachedRecordGroup& aRight)
              {
                  size_t minLength = std::min(aLeft.m_name.size(), aRight.m_name.size());
                  for (size_t i = 0; i != minLength; ++i)
                  {
                      char a = std::tolower(aLeft.m_name[i]);
                      char b = std::tolower(aRight.m_name[i]);
                      if (a != b)
                          return a < b;
                  }
                  return aLeft.m_name.size() < aRight.m_name.size();
              });
}

void TweakDBEditor::RefreshFlats()
{
    auto* pTDB = RED4ext::TweakDB::Get();
    constexpr uint64_t unknownGroupHash = RED4ext::FNV1a("!Unknown!");
    constexpr uint64_t badGroupHash = RED4ext::FNV1a("!BadName!");

    std::shared_lock<RED4ext::SharedMutex> _1(pTDB->mutex00);
    std::shared_lock<RED4ext::SharedMutex> _2(pTDB->mutex01);
    std::unordered_map<uint64_t, CachedFlatGroup> map;

    for (RED4ext::TweakDBID dbid : pTDB->flats)
    {
        const uint64_t dbidBase = m_vm.GetTDBIDBase(dbid);
        if (dbidBase != 0 && pTDB->recordsByID.Get(dbidBase) != nullptr)
            continue; // that's a record flat, ignoring that.

        std::string name;
        CachedFlatGroup* flatGroup = nullptr;
        if (!GetTweakDBIDStringFlat(dbid, name))
        {
            const auto it = map.find(unknownGroupHash);
            if (it == map.end())
                flatGroup = &map.emplace(unknownGroupHash, CachedFlatGroup("!Unknown!")).first->second;
            else
                flatGroup = &it->second;
        }
        else
        {
            size_t idx = name.find('.');
            if (idx == std::string::npos)
            {
                const auto it = map.find(badGroupHash);
                if (it == map.end())
                    flatGroup = &map.emplace(badGroupHash, CachedFlatGroup("!BadName!")).first->second;
                else
                    flatGroup = &it->second;
            }
            else
            {
                // < 1 || > size = group as much as possible
                for (int32_t i = 1; i != m_flatGroupNameDepth; ++i)
                {
                    if ((idx + 1) == name.size())
                        break;

                    size_t idx2 = name.find('.', idx + 1);
                    if (idx2 == std::string::npos)
                        break;

                    idx = idx2;
                }

                std::string groupName = name.substr(0, idx);
                uint64_t groupHash = RED4ext::FNV1a(groupName.c_str());
                const auto it = map.find(groupHash);
                if (it == map.end())
                    flatGroup = &map.emplace(groupHash, std::move(groupName)).first->second;
                else
                    flatGroup = &it->second;
            }
        }

        flatGroup->m_flats.emplace_back(std::move(name), dbid);
    }

    m_cachedFlatGroups.clear();
    m_cachedFlatGroups.reserve(map.size());
    for (auto& [groupHash, group] : map)
    {
        if (groupHash == unknownGroupHash || groupHash == badGroupHash)
            group.m_name = fmt::format("{} - {} flats!", group.m_name, group.m_flats.size());

        m_cachedFlatGroups.emplace_back(std::move(group));
    }

    // order groups
    std::sort(m_cachedFlatGroups.begin(), m_cachedFlatGroups.end(),
              [](const CachedFlatGroup& aLeft, const CachedFlatGroup& aRight)
              {
                  size_t minLength = std::min(aLeft.m_name.size(), aRight.m_name.size());
                  for (size_t i = 0; i != minLength; ++i)
                  {
                      char a = std::tolower(aLeft.m_name[i]);
                      char b = std::tolower(aRight.m_name[i]);
                      if (a != b)
                          return a < b;
                  }
                  return aLeft.m_name.size() < aRight.m_name.size();
              });
}

void TweakDBEditor::FilterAll()
{
    FilterRecords(true, true);
    FilterFlats();
}

RED4ext::TweakDBID ExtractTweakDBIDFromString(const char* acString)
{
    if (acString[0] == '\0')
        return 0;

    RED4ext::TweakDBID dbid = 0;
    if (sscanf(acString, "%llX", &dbid.value) != 1)
    {
        if (sscanf(acString, "<TDBID:%X:%hhX", &dbid.nameHash, &dbid.nameLength) != 2)
            (void)sscanf(acString, "ToTweakDBID{ hash = %X, length = %hhd", &dbid.nameHash, &dbid.nameLength);
    }

    return dbid;
}

void TweakDBEditor::FilterRecords(bool aFilterTab, bool aFilterDropdown)
{
    RED4ext::TweakDBID dbid = ExtractTweakDBIDFromString(s_recordsFilterBuffer);
    RED4ext::TweakDBID dbidDropdown = ExtractTweakDBIDFromString(s_tweakdbidFilterBuffer);
    for (CachedRecordGroup& group : m_cachedRecords)
    {
        bool anyRecordsVisible = false;
        for (CachedRecord& record : group.m_records)
        {
            if (aFilterTab)
            {
                if (s_recordsFilterBuffer[0] != '\0' && record.m_dbid != dbid &&
                    !StringContains(record.m_name, s_recordsFilterBuffer, s_recordsFilterIsRegex))
                {
                    record.m_isFiltered = true;
                }
                else
                {
                    record.m_isFiltered = false;
                    anyRecordsVisible = true;
                }
            }
            if (aFilterDropdown)
            {
                if (s_tweakdbidFilterBuffer[0] != '\0' && record.m_dbid != dbidDropdown &&
                    !StringContains(record.m_name, s_tweakdbidFilterBuffer))
                {
                    record.m_isDropdownFiltered = true;
                }
                else
                {
                    record.m_isDropdownFiltered = false;
                }
            }
        }
        if (aFilterTab)
        {
            group.m_isFiltered = !anyRecordsVisible;
        }
    }
}

void TweakDBEditor::FilterFlats()
{
    if (s_flatsFilterBuffer[0] == '\0')
    {
        for (CachedFlatGroup& group : m_cachedFlatGroups)
        {
            group.m_isFiltered = false;
            for (CachedFlat& flat : group.m_flats)
            {
                flat.m_isFiltered = false;
            }
        }
    }
    else
    {
        RED4ext::TweakDBID dbid = ExtractTweakDBIDFromString(s_flatsFilterBuffer);
        for (CachedFlatGroup& group : m_cachedFlatGroups)
        {
            bool anyFlatsVisible = false;
            for (CachedFlat& flat : group.m_flats)
            {
                if (flat.m_dbid != dbid &&
                    !StringContains(flat.m_name, s_flatsFilterBuffer, s_flatsFilterIsRegex))
                {
                    flat.m_isFiltered = true;
                }
                else
                {
                    flat.m_isFiltered = false;
                    anyFlatsVisible = true;
                }
            }

            group.m_isFiltered = !anyFlatsVisible;
        }
    }
}

bool TweakDBEditor::DrawRecordDropdown(const char* acpLabel, RED4ext::TweakDBID& aDBID, float aWidth)
{
    bool valueChanged = false;
    if (aWidth)
        ImGui::SetNextItemWidth(aWidth);
    std::string recordName = GetTweakDBIDStringRecord(aDBID);
    bool comboOpened = ImGui::BeginCombo(acpLabel, recordName.c_str(), ImGuiComboFlags_HeightLargest);
    if (ImGui::IsItemHovered())
    {
        RED4ext::Handle<RED4ext::IScriptable> record;
        if (RED4ext::TweakDB::Get()->TryGetRecord(aDBID, record))
        {
            RED4ext::CName typeName;
            record->GetType()->GetName(typeName);
            ImGui::SetTooltip(typeName.ToString());
        }
        else
        {
            ImGui::SetTooltip("ERROR_RECORD_NOT_FOUND");
        }
    }
    if (comboOpened)
    {
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputTextWithHint("##dropdownSearch", "Search", s_tweakdbidFilterBuffer,
                                     sizeof(s_tweakdbidFilterBuffer)))
        {
            FilterRecords(false, true);
        }

        if (ImGui::BeginChild("##dropdownScroll", ImVec2(0, g_comboDropdownHeight)))
        {
            for (CachedRecordGroup& recordGroup : m_cachedRecords)
            {
                for (CachedRecord& record : recordGroup.m_records)
                {
                    if (record.m_isDropdownFiltered)
                        continue;

                    bool isSelected = record.m_dbid == aDBID;
                    if (ImGui::NextItemVisible(ImVec2(1.0f, ImGui::GetTextLineHeight())) &&
                        ImGui::Selectable(record.m_name.c_str(), isSelected))
                    {
                        aDBID = record.m_dbid;
                        valueChanged = true;
                        ImGui::CloseCurrentPopup();
                        break;
                    }

                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(recordGroup.m_name.c_str());

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                if (valueChanged)
                    break;
            }
        }
        ImGui::EndChild();

        ImGui::EndCombo();
    }

    return valueChanged;
}

#pragma region TweakDBID to string

std::string TweakDBEditor::GetTweakDBIDStringRecord(RED4ext::TweakDBID aDBID)
{
    std::string name;
    GetTweakDBIDStringRecord(aDBID, name);
    return std::move(name);
}

bool TweakDBEditor::GetTweakDBIDStringRecord(RED4ext::TweakDBID aDBID, std::string& aString)
{
    if (CDPRTweakDBMetadata::Get()->GetRecordName(aDBID, aString))
        return true;

    aString = CET::Get().GetVM().GetTDBIDString(aDBID);
    return aString[0] != '<' || aString[aString.size() - 1] != '>';
}

std::string TweakDBEditor::GetTweakDBIDStringFlat(RED4ext::TweakDBID aDBID)
{
    std::string name;
    GetTweakDBIDStringFlat(aDBID, name);
    return std::move(name);
}

bool TweakDBEditor::GetTweakDBIDStringFlat(RED4ext::TweakDBID aDBID, std::string& aString)
{
    if (CDPRTweakDBMetadata::Get()->GetFlatName(aDBID, aString))
        return true;

    aString = CET::Get().GetVM().GetTDBIDString(aDBID);
    return aString[0] != '<' || aString[aString.size() - 1] != '>';
}

std::string TweakDBEditor::GetTweakDBIDStringQuery(RED4ext::TweakDBID aDBID)
{
    std::string name;
    GetTweakDBIDStringQuery(aDBID, name);
    return std::move(name);
}

bool TweakDBEditor::GetTweakDBIDStringQuery(RED4ext::TweakDBID aDBID, std::string& aString)
{
    if (CDPRTweakDBMetadata::Get()->GetQueryName(aDBID, aString))
        return true;

    aString = CET::Get().GetVM().GetTDBIDString(aDBID);
    return aString[0] != '<' || aString[aString.size() - 1] != '>';
}

#pragma endregion

#pragma region Drawing flats

bool TweakDBEditor::DrawFlat(RED4ext::TweakDBID aDBID)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    auto* pFlatValue = pTDB->GetFlatValue(aDBID);
    if (pFlatValue == nullptr)
    {
        ImGui::Text("'%s' is not found in TweakDB", GetTweakDBIDStringFlat(aDBID.value & 0xFFFFFFFFFF).c_str());
        return false;
    }

    RED4ext::CStackType stackType = pFlatValue->GetValue();

    ImGui::PushID(aDBID.nameHash);
    ImGui::PushID(aDBID.nameLength);
    bool isModified = DrawFlat(aDBID, stackType);
    ImGui::PopID();
    ImGui::PopID();

    return isModified;
}

bool TweakDBEditor::DrawFlat(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    static auto* pRTTI = RED4ext::CRTTISystem::Get();
    static auto* pTweakDBIDType = pRTTI->GetType("TweakDBID");
    static auto* pQuaternionType = pRTTI->GetType("Quaternion");
    static auto* pEulerAnglesType = pRTTI->GetType("EulerAngles");
    static auto* pVector3Type = pRTTI->GetType("Vector3");
    static auto* pVector2Type = pRTTI->GetType("Vector2");
    static auto* pColorType = pRTTI->GetType("Color");
    static auto* pGamedataLocKeyWrapperType = pRTTI->GetType("gamedataLocKeyWrapper");
    static auto* pRaRefCResourceType = pRTTI->GetType("raRef:CResource");
    static auto* pCNameType = pRTTI->GetType("CName");
    static auto* pBoolType = pRTTI->GetType("Bool");
    static auto* pStringType = pRTTI->GetType("String");
    static auto* pFloatType = pRTTI->GetType("Float");
    static auto* pInt32Type = pRTTI->GetType("Int32");

    if (aStackType.type->GetType() == RED4ext::ERTTIType::Array)
        return DrawFlatArray(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pTweakDBIDType)
        return DrawFlatTweakDBID(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pQuaternionType)
        return DrawFlatQuaternion(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pEulerAnglesType)
        return DrawFlatEulerAngles(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pVector3Type)
        return DrawFlatVector3(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pVector2Type)
        return DrawFlatVector2(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pColorType)
        return DrawFlatColor(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pGamedataLocKeyWrapperType)
        return DrawFlatLocKeyWrapper(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pRaRefCResourceType)
        return DrawFlatResourceAsyncRef(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pCNameType)
        return DrawFlatCName(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pBoolType)
        return DrawFlatBool(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pStringType)
        return DrawFlatString(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pFloatType)
        return DrawFlatFloat(aDBID, aStackType, aReadOnly);
    else if (aStackType.type == pInt32Type)
        return DrawFlatInt32(aDBID, aStackType, aReadOnly);

    RED4ext::CName typeName;
    aStackType.type->GetName(typeName);
    ImGui::Text("unsupported type: %s", typeName.ToString());
    return false;
}

// Needs a refactor
bool TweakDBEditor::DrawFlatArray(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly,
                                  bool aCollapsable)
{
    static std::unordered_map<uint64_t, RED4ext::ScriptInstance> editedArrays;

    auto* pArrayType = reinterpret_cast<RED4ext::CArray*>(aStackType.type);
    auto* pArrayInnerType = pArrayType->GetInnerType();
    RED4ext::CName arrayTypeName;
    pArrayType->GetName(arrayTypeName);

    bool isModified = false;
    bool isCachable = !aReadOnly && aDBID.IsValid();
    bool isCached = false; // if it's currently in 'editedArrays'

    RED4ext::ScriptInstance arrayInstance = aStackType.value;
    if (isCachable)
    {
        const auto it = editedArrays.find(aDBID.value & 0xFFFFFFFFFF);
        if (it != editedArrays.end())
        {
            isCached = true;
            arrayInstance = it->second;
        }
        else
        {
            aReadOnly = true;
        }
    }

    uint32_t arraySize = pArrayType->GetLength(arrayInstance);
    if (!aCollapsable || ImGui::TreeNode("", "[%s] %u items", arrayTypeName.ToString(), arraySize))
    {
        if (!aReadOnly && ImGui::Button("clear"))
        {
            pArrayType->Resize(arrayInstance, 0);
            arraySize = 0;
        }

        if (isCachable)
        {
            uint64_t arrayKey = aDBID.value & 0xFFFFFFFFFF;
            if (!isCached)
            {
                if (ImGui::Button("edit"))
                {
                    RED4ext::IMemoryAllocator* allocator = pArrayType->GetAllocator();
                    auto result = allocator->AllocAligned(pArrayType->GetSize(), pArrayType->GetAlignment());
                    pArrayType->Init(result.memory);
                    pArrayType->Assign(result.memory, arrayInstance);
                    editedArrays.emplace(arrayKey, result.memory);
                }
            }
            else
            {
                ImGui::SameLine();
                if (ImGui::Button("cancel"))
                {
                    pArrayType->Destroy(arrayInstance);
                    pArrayType->GetAllocator()->Free(arrayInstance);
                    editedArrays.erase(arrayKey);
                    // arraySize = 0;

                    // ugh.. UI freaks out if you don't render
                    arrayInstance = aStackType.value;
                    arraySize = pArrayType->GetLength(arrayInstance);
                    isCached = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("save"))
                {
                    RED4ext::CStackType newStackType(aStackType.type, arrayInstance);
                    isModified = TweakDB::InternalSetFlat(aDBID, newStackType);

                    pArrayType->Destroy(arrayInstance);
                    pArrayType->GetAllocator()->Free(arrayInstance);
                    editedArrays.erase(arrayKey);
                    // arraySize = 0;

                    // ugh.. UI freaks out if you don't render
                    arrayInstance = aStackType.value;
                    arraySize = pArrayType->GetLength(arrayInstance);
                    isCached = false;
                }
            }
        }

        static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders |
                                                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg;

        if (isCached)
        {
            // set background to red when the array is being edited
            ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(1.00f, 0.00f, 0.00f, 0.06f));
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(1.00f, 0.00f, 0.00f, 0.12f));
        }

        // TODO: fix column resizing issue on first frame
        if (ImGui::BeginTable("arrayElements", 2, tableFlags))
        {
            uint32_t deleteElementIdx = -1;
            for (uint32_t i = 0; i != arraySize; ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::PushID(i);

                if (!aReadOnly)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.00f, 0.00f, 1.00f));
                    if (ImGui::Button("-", ImVec2(20.0f, 20.0f)))
                    {
                        deleteElementIdx = i;
                    }
                    ImGui::PopStyleColor(1);
                    ImGui::SameLine();
                }

                ImGui::Text("[%u]", i);
                ImGui::TableNextColumn();

                RED4ext::CStackType stackType;
                stackType.type = pArrayInnerType;
                stackType.value = pArrayType->GetElement(arrayInstance, i);
                bool flatModified = DrawFlat({}, stackType, aReadOnly);
                // cached arrays' isModified is managed by the save button
                if (!isCached)
                    isModified |= flatModified;

                ImGui::PopID();
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (!aReadOnly && ImGui::Button("add new"))
            {
                pArrayType->InsertAt(arrayInstance, arraySize);
                pArrayInnerType->Init(pArrayType->GetElement(arrayInstance, arraySize));
            }

            ImGui::EndTable();

            if (deleteElementIdx != -1)
            {
                pArrayType->RemoveAt(arrayInstance, deleteElementIdx);
            }
        }

        if (isCached)
        {
            ImGui::PopStyleColor(2);
        }

        if (aCollapsable)
            ImGui::TreePop();
    }

    return isModified;
}

bool TweakDBEditor::DrawFlatTweakDBID(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pDBID = reinterpret_cast<RED4ext::TweakDBID*>(aStackType.value);

    if (aReadOnly)
    {
        ImGui::SetNextItemWidth(-1);
        std::string recordName = GetTweakDBIDStringRecord(*pDBID);
        ImGui::InputText("", recordName.data(), recordName.size(), ImGuiInputTextFlags_ReadOnly);

        if (ImGui::IsItemHovered())
        {
            RED4ext::Handle<RED4ext::IScriptable> record;
            if (pDBID->IsValid() && RED4ext::TweakDB::Get()->TryGetRecord(pDBID->value, record))
            {
                RED4ext::CName typeName;
                record->GetType()->GetName(typeName);
                ImGui::SetTooltip(typeName.ToString());
            }
            else
            {
                ImGui::SetTooltip("ERROR_RECORD_NOT_FOUND");
            }
        }

        ImGui::SetNextItemWidth(-1);
        ImGui::InputScalar("##raw", ImGuiDataType_U64, aStackType.value, nullptr, nullptr, "%016llX",
                           ImGuiInputTextFlags_ReadOnly);
    }
    else
    {
        RED4ext::TweakDBID dbid = *pDBID;
        bool valueChanged = DrawRecordDropdown("", dbid, -1);

        int32_t flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal;
        ImGui::SetNextItemWidth(-1);
        valueChanged |= ImGui::InputScalar("##raw", ImGuiDataType_U64, &dbid.value, nullptr, nullptr, "%016llX", flags);

        if (valueChanged)
        {
            if (aDBID.IsValid())
            {
                RED4ext::CStackType newStackType(aStackType.type, &dbid);
                return TweakDB::InternalSetFlat(aDBID, newStackType);
            }
            else
            {
                aStackType.type->Move(aStackType.value, &dbid);
                return true;
            }
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatQuaternion(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pQuat = reinterpret_cast<RED4ext::Quaternion*>(aStackType.value);

    float i = pQuat->i;
    float j = pQuat->j;
    float k = pQuat->k;
    float r = pQuat->r;

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::Text("I");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputFloat("##I", &i, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("J");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##J", &j, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("K");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##K", &k, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("R");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##R", &r, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Quaternion newQuat;
        newQuat.i = i;
        newQuat.j = j;
        newQuat.k = k;
        newQuat.r = r;

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newQuat);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newQuat);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatEulerAngles(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pEular = reinterpret_cast<RED4ext::EulerAngles*>(aStackType.value);

    float roll = pEular->Roll;
    float pitch = pEular->Pitch;
    float yaw = pEular->Yaw;

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::Text("Roll ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputFloat("##Roll", &roll, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("Pitch");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##Pitch", &pitch, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("Yaw  ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##Yaw", &yaw, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::EulerAngles newEular;
        newEular.Roll = roll;
        newEular.Pitch = pitch;
        newEular.Yaw = yaw;

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newEular);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newEular);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatVector3(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pVec = reinterpret_cast<RED4ext::Vector3*>(aStackType.value);

    float x = pVec->X;
    float y = pVec->Y;
    float z = pVec->Z;

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputFloat("##X", &x, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##Y", &y, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("Z");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##Z", &z, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Vector3 newVec;
        newVec.X = x;
        newVec.Y = y;
        newVec.Z = z;

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newVec);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newVec);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatVector2(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pVec = reinterpret_cast<RED4ext::Vector2*>(aStackType.value);

    float x = pVec->X;
    float y = pVec->Y;

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputFloat("##X", &x, 0.0f, 0.0f, "%f", flags);

    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    valueChanged |= ImGui::InputFloat("##Y", &y, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Vector2 newVec;
        newVec.X = x;
        newVec.Y = y;

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newVec);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newVec);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatColor(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pColor = reinterpret_cast<RED4ext::Color*>(aStackType.value);

    float rgba[4];
    rgba[0] = pColor->Red / 255.0f;
    rgba[1] = pColor->Green / 255.0f;
    rgba[2] = pColor->Blue / 255.0f;
    rgba[3] = pColor->Alpha / 255.0f;

    aReadOnly = true;
    ImGui::Text("'Color' is not supported yet");
    ImGui::SameLine();

    int32_t flags = aReadOnly ? ImGuiColorEditFlags_NoInputs : ImGuiColorEditFlags_None;
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::ColorEdit4("", rgba, flags | ImGuiColorEditFlags_AlphaPreview);
    // Color picker returns true everytime it changes
    // It will overkill the FlatValuePool
    // Thankfully, I don't think 'Color' is used
    valueChanged = false;

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Color newColor;
        newColor.Red = std::clamp(rgba[0], 0.0f, 1.0f) * 255;
        newColor.Green = std::clamp(rgba[1], 0.0f, 1.0f) * 255;
        newColor.Blue = std::clamp(rgba[2], 0.0f, 1.0f) * 255;
        newColor.Alpha = std::clamp(rgba[3], 0.0f, 1.0f) * 255;

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newColor);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newColor);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatLocKeyWrapper(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pLocKey = reinterpret_cast<RED4ext::gamedataLocKeyWrapper*>(aStackType.value);

    ImGui::Text("This is a LocalizationKey");
    ImGui::Text("Game.GetLocalizedTextByKey(...)");

    uint64_t key = pLocKey->unk00;
    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputScalar("", ImGuiDataType_U64, &key, nullptr, nullptr, nullptr, flags);

    {
        RED4ext::CString localizedText;
        RED4ext::ExecuteGlobalFunction("GetLocalizedTextByKey", &localizedText, key);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##string", (char*)localizedText.c_str(), localizedText.Length(),
                         ImGuiInputTextFlags_ReadOnly);
    }

    if (!aReadOnly && valueChanged)
    {
        RED4ext::gamedataLocKeyWrapper newLocKey;
        newLocKey.unk00 = key;

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newLocKey);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newLocKey);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatResourceAsyncRef(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pRaRef = reinterpret_cast<RED4ext::ResourceAsyncReference<void>*>(aStackType.value);

    uint64_t hashRef = reinterpret_cast<uint64_t>(pRaRef->ref);
    ImGui::Text("raRef:CResource is not supported yet");
    ImGui::Text("but you can modify the hash!");

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    flags |= ImGuiInputTextFlags_CharsHexadecimal;
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputScalar("", ImGuiDataType_U64, &hashRef, nullptr, nullptr, "%016llX", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::ResourceAsyncReference<void> newRaRef;
        newRaRef.ref = reinterpret_cast<void*>(hashRef);

        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newRaRef);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newRaRef);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatCName(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pCName = reinterpret_cast<RED4ext::CName*>(aStackType.value);

    ImGui::Text("This is not just a string.");
    ImGui::Text("Game is expecting specific values.");
    // Is it worth it to implement a dropdown like DrawTweakDBID?

    RED4ext::CName newCName;
    bool valueChanged = false;
    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    // Causes issues with 'InputTextCStr' when you write a bad value and try to undo
    flags |= ImGuiInputTextFlags_NoUndoRedo;

    auto* pStr = pCName->ToString();
    ImGui::SetNextItemWidth(-1);
    auto [strChanged, pModifiedStr] = ImGui::InputTextCStr("", pStr, strlen(pStr), flags);
    if (strChanged)
    {
        if (stricmp(pModifiedStr->c_str(), "none") == 0)
            newCName.hash = 0;
        else
            newCName = RED4ext::CName(pModifiedStr->c_str());
        valueChanged = newCName.hash != pCName->hash;
    }

    if (ImGui::IsItemHovered() && strnicmp(pStr, "LocKey#", 7) == 0)
    {
        RED4ext::CString localizedText;
        RED4ext::ExecuteGlobalFunction("GetLocalizedTextByKey", &localizedText, atoll(pStr + 7));
        ImGui::SetTooltip(localizedText.c_str());
    }

    uint64_t hash = pCName->hash;
    ImGui::SetNextItemWidth(-1);
    bool rawChanged = ImGui::InputScalar("##raw", ImGuiDataType_U64, &hash, nullptr, nullptr, "%016llX",
                                         flags | ImGuiInputTextFlags_CharsHexadecimal);
    if (rawChanged)
    {
        newCName.hash = hash;
        valueChanged = true;
    }

    if (valueChanged)
    {
        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newCName);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newCName);
            return true;
        }
    }

    return valueChanged;
}

bool TweakDBEditor::DrawFlatBool(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pBool = reinterpret_cast<bool*>(aStackType.value);

    bool val = *pBool;
    bool valueChanged = ImGui::Checkbox("", &val);
    if (!aReadOnly && valueChanged)
    {
        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &val);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &val);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatString(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pCString = reinterpret_cast<RED4ext::CString*>(aStackType.value);

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::SetNextItemWidth(-1);
    auto [valueChanged, pModifiedStr] = ImGui::InputTextCStr("", pCString->c_str(), pCString->Length(), flags);
    if (ImGui::IsItemHovered() && strnicmp(pCString->c_str(), "LocKey#", 7) == 0)
    {
        RED4ext::CString localizedText;
        RED4ext::ExecuteGlobalFunction("GetLocalizedTextByKey", &localizedText, atoll(pCString->c_str() + 7));
        ImGui::SetTooltip(localizedText.c_str());
    }

    if (valueChanged)
    {
        RED4ext::CString newCString(pModifiedStr->c_str());
        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &newCString);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &newCString);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatFloat(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pFloat = reinterpret_cast<float*>(aStackType.value);

    float val = *pFloat;
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputFloat(
        "", &val, 0, 0, "%f", aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue);
    if (valueChanged)
    {
        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &val);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &val);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatInt32(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    auto* pInt = reinterpret_cast<int32_t*>(aStackType.value);

    int32_t val = *pInt;
    ImGui::SetNextItemWidth(-1);
    bool valueChanged = ImGui::InputInt(
        "", &val, 0, 0, aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue);
    if (valueChanged)
    {
        if (aDBID.IsValid())
        {
            RED4ext::CStackType newStackType(aStackType.type, &val);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }
        else
        {
            aStackType.type->Move(aStackType.value, &val);
            return true;
        }
    }

    return false;
}

#pragma endregion

void TweakDBEditor::DrawRecordsTab()
{
    auto* pTDB = RED4ext::TweakDB::Get();

    ImGui::SetNextItemWidth(-70);
    if (ImGui::InputTextWithHint("##search", "Search", s_recordsFilterBuffer, sizeof(s_recordsFilterBuffer)))
    {
        FilterRecords();
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Regex", &s_recordsFilterIsRegex))
    {
        FilterRecords();
    }

    if (!ImGui::BeginChild("##scrollable"))
        ImGui::EndChild();

    for (CachedRecordGroup& group : m_cachedRecords)
    {
        if (group.m_isFiltered || !group.m_visibilityChecker.IsVisible())
            continue;

        group.m_visibilityChecker.Begin();

        if (ImGui::CollapsingHeader(group.m_name.c_str()))
        {
            for (CachedRecord& record : group.m_records)
            {
                if (record.m_isFiltered || !record.m_visibilityChecker.IsVisible())
                    continue;

                record.m_visibilityChecker.Begin();

                if (ImGui::TreeNode(record.m_name.c_str()))
                {
                    static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings |
                                                                  ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable |
                                                                  ImGuiTableFlags_RowBg;

                    if (!record.m_initialized)
                        record.Initialize();

                    if (ImGui::BeginTable("flatsTable", 2, tableFlags))
                    {
                        for (CachedFlat& flat : record.m_flats)
                        {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();

                            if (!flat.m_visibilityChecker.IsVisible())
                                continue;
                            flat.m_visibilityChecker.Begin();

                            flat.Update();

                            ImGui::PushID(flat.m_name.c_str());
                            ImGui::SetNextItemWidth(-1);
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
                            ImGui::InputText("", flat.m_name.data(), flat.m_name.size(), ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopStyleColor();
                            ImGui::PopID();

                            ImGui::TableNextColumn();
                            if (flat.m_isMissing)
                            {
                                ImGui::Text("ERROR_FLAT_NOT_FOUND");
                            }
                            else
                            {
                                if (DrawFlat(flat.m_dbid))
                                {
                                    record.Update();
                                }
                            }

                            flat.m_visibilityChecker.End();
                        }
                        ImGui::EndTable();
                    }

                    ImGui::TreePop();
                }

                record.m_visibilityChecker.End();
            }
        }

        group.m_visibilityChecker.End();
    }

    ImGui::EndChild();
}

void TweakDBEditor::DrawQueriesTab()
{
    auto* pTDB = RED4ext::TweakDB::Get();

    std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex01);
    pTDB->queries.ForEach([this](const RED4ext::TweakDBID& queryID, RED4ext::DynArray<RED4ext::TweakDBID>& recordIDs)
    {
        auto queryName = GetTweakDBIDStringQuery(queryID.value);

        ImGui::PushID(queryID.nameHash);
        ImGui::PushID(queryID.nameLength);
        if (ImGui::CollapsingHeader(queryName.c_str()))
        {
            static RED4ext::CStackType stackType(RED4ext::CRTTISystem::Get()->GetType("array:TweakDBID"));
            stackType.value = &recordIDs;
            DrawFlatArray({}, stackType, false, false);
        }
        ImGui::PopID();
        ImGui::PopID();
    });
}

void TweakDBEditor::DrawFlatsTab()
{
    ImGui::SetNextItemWidth(-70);
    if (ImGui::InputTextWithHint("##search", "Search", s_flatsFilterBuffer, sizeof(s_flatsFilterBuffer)))
    {
        FilterFlats();
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Regex", &s_flatsFilterIsRegex))
    {
        FilterFlats();
    }

    if (!ImGui::BeginChild("##scrollable"))
        ImGui::EndChild();

    for (CachedFlatGroup& group : m_cachedFlatGroups)
    {
        if (group.m_isFiltered || !group.m_visibilityChecker.IsVisible())
            continue;

        group.m_visibilityChecker.Begin();

        if (ImGui::CollapsingHeader(group.m_name.c_str()))
        {
            static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders |
                                                          ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg;

            if (!group.m_initialized)
                group.Initialize();

            if (ImGui::BeginTable("flatsTable", 2, tableFlags))
            {
                for (CachedFlat& flat : group.m_flats)
                {
                    if (flat.m_isFiltered)
                        continue;

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    if (!flat.m_visibilityChecker.IsVisible())
                        continue;
                    flat.m_visibilityChecker.Begin();

                    flat.Update();

                    ImGui::PushID(flat.m_name.c_str());
                    ImGui::SetNextItemWidth(-1);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
                    ImGui::InputText("", flat.m_name.data(), flat.m_name.size(), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopStyleColor();
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    if (flat.m_isMissing)
                    {
                        ImGui::Text("ERROR_FLAT_NOT_FOUND");
                    }
                    else
                    {
                        DrawFlat(flat.m_dbid);
                    }

                    flat.m_visibilityChecker.End();
                }
                ImGui::EndTable();
            }
        }

        group.m_visibilityChecker.End();
    }

    ImGui::EndChild();
}

void TweakDBEditor::DrawAdvancedTab()
{
    if (CDPRTweakDBMetadata::Get()->IsInitialized())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF00FF00);
        ImGui::Text("'tweakdb.str' is loaded!");
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFF0000FF);
        ImGui::Text("'tweakdb.str' is not loaded.");
        ImGui::PopStyleColor();
        ImGui::TreePush();
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
        ImGui::PushStyleColor(ImGuiCol_Text, 0xFFF66409);
        char* pLink = const_cast<char*>("https://www.cyberpunk.net/en/modding-support");
        ImGui::InputText("##cdprLink", pLink, 44, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor(2);
        ImGui::Text("1) Download and unpack 'Metadata'");
        ImGui::Text("2) Copy 'tweakdb.str' to 'plugins\\cyber_engine_tweaks\\tweakdb.str'");
        std::string cetDir = CET::Get().GetPaths().CETRoot().string();
        ImGui::Text("Full path: %s", cetDir.c_str());
        if (ImGui::Button("3) Load tweakdb.str"))
        {
            if (CDPRTweakDBMetadata::Get()->Initialize())
            {
                RefreshAll();
                FilterAll();
            }
        }
        ImGui::TreePop();
    }

    ImGui::SetNextItemWidth(50);
    if (ImGui::InputScalar("'Flats' Grouping depth", ImGuiDataType_S8, &m_flatGroupNameDepth, nullptr, nullptr, nullptr,
                           ImGuiInputTextFlags_EnterReturnsTrue))
    {
        RefreshFlats();
        FilterFlats();
    }
    ImGui::SetNextItemWidth(100);
    ImGui::InputFloat("ComboBox dropdown height", &g_comboDropdownHeight, 0, 0);

    if (ImGui::Button("Refresh all"))
    {
        RefreshAll();
        FilterAll();
    }

    if (ImGui::BeginChild("CreateRecord", ImVec2(0.0f, 170.0f), true))
    {
        static const char* status = "";
        static float statusTimer = 0.0f;
        static char recordName[256]{};
        static char comboSearchBuffer[70]{};
        static RED4ext::TweakDBID clonedRecordDBID;
        static RED4ext::CName recordTypeName;

        constexpr auto SetStatus = [](const char* acpStatus) {
            status = acpStatus;
            statusTimer = 2.5f;
        };

        ImGui::InputText("Record name", recordName, sizeof(recordName));

        if (ImGui::Button("Delete Record"))
        {
            if (TweakDB::InternalDeleteRecord(RED4ext::TweakDBID(recordName), spdlog::get("scripting")))
                SetStatus("Success!");
            else
                SetStatus("Failed. check console!");
        }

        if (ImGui::BeginCombo("Record type to create", recordTypeName.ToString(), ImGuiComboFlags_HeightLargest))
        {
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##dropdownSearch", "Search", comboSearchBuffer, sizeof(comboSearchBuffer));
            if (ImGui::BeginChild("##dropdownScroll", ImVec2(0, g_comboDropdownHeight)))
            {
                for (CachedRecordGroup& recordGroup : m_cachedRecords)
                {
                    if (comboSearchBuffer[0] != '\0' &&
                        !StringContains(recordGroup.m_name, comboSearchBuffer))
                    {
                        continue;
                    }

                    bool isSelected = recordTypeName == recordGroup.m_typeName;
                    if (ImGui::NextItemVisible(ImVec2(1.0f, ImGui::GetTextLineHeight())) &&
                        ImGui::Selectable(recordGroup.m_name.c_str(), isSelected))
                    {
                        recordTypeName = recordGroup.m_typeName;
                        ImGui::CloseCurrentPopup();
                        break;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndChild();

            ImGui::EndCombo();
        }

        if (ImGui::Button("Create record"))
        {
            if (TweakDB::InternalCreateRecord(recordName, recordTypeName.ToString(), spdlog::get("scripting")))
                SetStatus("Success!");
            else
                SetStatus("Failed. check console!");
        }

        DrawRecordDropdown("Record to clone", clonedRecordDBID);

        if (ImGui::Button("Clone record"))
        {
            if (TweakDB::InternalCloneRecord(recordName, clonedRecordDBID, spdlog::get("scripting")))
                SetStatus("Success!");
            else
                SetStatus("Failed. check console!");
        }

        if (statusTimer)
        {
            statusTimer -= ImGui::GetIO().DeltaTime;
            if (statusTimer <= 0.0f)
            {
                status = "";
                statusTimer = 0.0f;
            }
        }
        ImGui::Text(status);
    }
    ImGui::EndChild();
}

TweakDBEditor::CachedFlat::CachedFlat(std::string aName, RED4ext::TweakDBID aDBID) noexcept
    : m_name(std::move(aName))
    , m_dbid(aDBID)
{
}

void TweakDBEditor::CachedFlat::Update(int32_t aTDBOffset)
{
    auto* pTDB = RED4ext::TweakDB::Get();

    if (aTDBOffset == -1)
    {
        std::shared_lock<RED4ext::SharedMutex> _(pTDB->mutex00);
        const auto it = pTDB->flats.Find(m_dbid);
        if (it != pTDB->flats.end())
        {
            m_dbid.value = it->value;
            m_isMissing = false;
        }
        else
        {
            m_isMissing = true;
        }
    }
    else
    {
        m_dbid.SetTDBOffset(aTDBOffset);
    }
}

TweakDBEditor::CachedFlatGroup::CachedFlatGroup(std::string aName) noexcept
    : m_name(std::move(aName))
{
}

void TweakDBEditor::CachedFlatGroup::Initialize()
{
    if (m_initialized)
        return;

    // order flats inside group
    std::sort(m_flats.begin(), m_flats.end(), [](const CachedFlat& aLeft, const CachedFlat& aRight)
    {
        return SortTweakDBIDName(aLeft.m_name, aRight.m_name);
    });

    m_initialized = true;
}

TweakDBEditor::CachedRecord::CachedRecord(std::string aName, RED4ext::TweakDBID aDBID) noexcept
    : m_name(std::move(aName))
    , m_dbid(aDBID)
{
}

void TweakDBEditor::CachedRecord::Initialize()
{
    if (m_initialized)
        return;

    std::vector<uint64_t> recordFlats;
    CET::Get().GetVM().GetTDBIDDerivedFrom(m_dbid.value, recordFlats);
    if (!recordFlats.empty())
    {
        for (uint64_t flatID : recordFlats)
        {
            m_flats.emplace_back(TweakDBEditor::GetTweakDBIDStringFlat(flatID), flatID);
        }
    }

    std::sort(m_flats.begin(), m_flats.end(), [](const CachedFlat& aLeft, const CachedFlat& aRight)
    {
        return SortTweakDBIDName(aLeft.m_name, aRight.m_name);
    });

    m_initialized = true;
}

void TweakDBEditor::CachedRecord::Update()
{
    auto* pTDB = RED4ext::TweakDB::Get();
    pTDB->UpdateRecord(m_dbid);
}

TweakDBEditor::CachedRecordGroup::CachedRecordGroup(RED4ext::CName aTypeName)
    : m_typeName(aTypeName)
    , m_name(aTypeName.ToString())
{
}

bool TweakDBEditor::ImGuiVisibilityChecker::IsVisible(bool aClaimSpaceIfInvisible)
{
    return m_itemSize.y == 0 || ImGui::NextItemVisible(m_itemSize, aClaimSpaceIfInvisible);
}

void TweakDBEditor::ImGuiVisibilityChecker::Begin()
{
    m_beginCursorY = ImGui::GetCursorPosY();
}

void TweakDBEditor::ImGuiVisibilityChecker::End()
{
    float endCursorY = ImGui::GetCursorPosY();
    if (endCursorY < m_beginCursorY)
    {
        m_itemSize = ImVec2(0.0f, 0.0f);
    }
    else
    {
        m_itemSize = ImVec2(ImGui::GetContentRegionMax().x, endCursorY - m_beginCursorY);
        m_itemSize.y -= ImGui::GetStyle().ItemSpacing.y;
    }
}
