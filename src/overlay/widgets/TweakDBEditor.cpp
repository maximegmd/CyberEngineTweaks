#include <stdafx.h>

#if GAME_CYBERPUNK

#include "TweakDBEditor.h"

#include "EngineTweaks.h"
#include <Utils.h>

#include <reverse/TweakDB/ResourcesList.h>
#include <reverse/TweakDB/TweakDB.h>

#include <RED4ext/Scripting/Natives/Generated/Color.hpp>
#include <RED4ext/Scripting/Natives/Generated/EulerAngles.hpp>
#include <RED4ext/Scripting/Natives/Generated/Quaternion.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector2.hpp>
#include <RED4ext/Scripting/Natives/Generated/Vector3.hpp>

bool TweakDBEditor::s_recordsFilterIsRegex = false;
bool TweakDBEditor::s_flatsFilterIsRegex = false;
char TweakDBEditor::s_recordsFilterBuffer[256]{};
char TweakDBEditor::s_flatsFilterBuffer[256]{};
char TweakDBEditor::s_tweakdbidFilterBuffer[256]{};
float g_comboDropdownHeight = 300.0f;
constexpr float c_searchDelay = 0.25f;

namespace ImGui
{
std::pair<bool, std::string*> InputTextCStr(const char* acpLabel, const char* acpBuf, size_t aBufSize, ImGuiInputTextFlags aFlags = 0)
{
    if ((aFlags & ImGuiInputTextFlags_ReadOnly) == 0)
    {
        static bool isModified;
        static std::string tempStr;

        isModified = false;
        tempStr.clear();
        aFlags |= ImGuiInputTextFlags_CallbackResize;
        const bool ret = InputText(
            acpLabel, const_cast<char*>(acpBuf), aBufSize + 1, aFlags,
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

    InputText(acpLabel, const_cast<char*>(acpBuf), aBufSize, aFlags);
    return {false, nullptr};
}

void SetTooltipUnformatted(const char* acpText)
{
    BeginTooltip();
    TextUnformatted(acpText);
    EndTooltip();
}

bool NextItemVisible(const ImVec2& aSize = ImVec2(1, 1), bool aClaimSpaceIfInvisible = true)
{
    const ImVec2 rectMin = GetCursorScreenPos();
    const auto rectMax = ImVec2(rectMin.x + aSize.x, rectMin.y + aSize.y);
    const bool visible = IsRectVisible(rectMin, rectMax);
    if (!visible && aClaimSpaceIfInvisible)
    {
        Dummy(aSize);
    }
    return visible;
}
} // namespace ImGui

bool SortString(const std::string& acLeft, const std::string& acRight)
{
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

bool SortTweakDBIDString(const std::string& acLeft, const std::string& acRight)
{
    // unknown TweakDBID should be at the bottom
    if (acLeft[0] == '<' && acRight[0] != '<')
        return false;

    if (acLeft[0] != '<' && acRight[0] == '<')
        return true;

    return std::ranges::lexicographical_compare(acLeft, acRight);
}

bool StringContains(const std::string_view& acString, const std::string_view& acSearch, bool aRegex = false)
{
    if (acSearch.empty())
        return false;

    if (aRegex)
    {
        try
        {
            const std::regex searchRegex(acSearch.begin(), acSearch.end());
            return std::regex_search(acString.begin(), acString.end(), searchRegex);
        }
        catch (std::regex_error&)
        {
            return false;
        }
    }

    const auto it = std::ranges::search(acString, acSearch, [](char a, char b) { return std::tolower(a) == std::tolower(b); }).begin();
    return it != acString.end();
}

TweakDBEditor::TweakDBEditor(LuaVM& aVm)
    : Widget("TweakDB Editor")
    , m_vm(aVm)
{
}

void TweakDBEditor::OnUpdate()
{
    // LuaVM is initialized after TweakDB, let's wait for it
    if (!m_vm.IsInitialized())
    {
        ImGui::TextUnformatted("TweakDB is not initialized yet");
        return;
    }

    if (!m_initialized)
    {
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

    std::shared_lock _(pTDB->mutex01);

    m_cachedRecords.clear();
    m_cachedRecords.reserve(pTDB->recordsByType.size);

    pTDB->recordsByType.for_each(
        [this](const RED4ext::CBaseRTTIType* acpRTTIType, const RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>>& aRecords)
        {
            const auto typeName = acpRTTIType->GetName();
            auto& groupRecords = m_cachedRecords.emplace_back(typeName).m_records;

            groupRecords.reserve(groupRecords.size() + aRecords.size);
            for (auto& handle : aRecords)
            {
                const auto* record = reinterpret_cast<RED4ext::gamedataTweakDBRecord*>(handle.GetPtr());
                std::string recordName = GetTweakDBIDStringRecord(record->recordID);
                if (TweakDB::IsACreatedRecord(record->recordID))
                    recordName.insert(0, "* ");
                CachedRecord cachedRecord(std::move(recordName), record->recordID);
                cachedRecord.InitializeFlats();
                groupRecords.emplace_back(std::move(cachedRecord));
            }
        });

    std::ranges::sort(m_cachedRecords, [](const CachedRecordGroup& acLeft, const CachedRecordGroup& acRight) { return SortString(acLeft.m_name, acRight.m_name); });
}

void TweakDBEditor::RefreshFlats()
{
    auto* pTDB = RED4ext::TweakDB::Get();
    constexpr uint64_t unknownGroupHash = RED4ext::FNV1a64("!Unknown!");
    constexpr uint64_t badGroupHash = RED4ext::FNV1a64("!BadName!");

    std::shared_lock _1(pTDB->mutex00);
    std::shared_lock _2(pTDB->mutex01);

    TiltedPhoques::Map<uint64_t, size_t> cachedFlatGroupsMap;
    auto cacheFlat = [&](uint64_t groupHash, const char* groupName, std::string&& name, RED4ext::TweakDBID dbid)
    {
        size_t index = 0;

        if (const auto it = cachedFlatGroupsMap.find(groupHash); it != cachedFlatGroupsMap.cend())
            index = it.value();
        else
        {
            index = m_cachedFlatGroups.size();

            m_cachedFlatGroups.emplace_back(groupName);
            cachedFlatGroupsMap.emplace(groupHash, index);
        }

        m_cachedFlatGroups[index].m_flats.emplace_back(std::move(name), dbid);
    };

    m_cachedFlatGroups.clear();

    std::ranges::for_each(
        pTDB->flats,
        [&](RED4ext::TweakDBID dbid)
        {
            const uint64_t dbidBase = m_vm.GetTDBIDBase(dbid);
            if (dbidBase != 0 && pTDB->recordsByID.Get(dbidBase) != nullptr)
                return; // that's a record flat, ignoring that.

            std::string flatName;
            const bool unknownFlatName = !GetTweakDBIDStringFlat(dbid, flatName);
            if (unknownFlatName)
                cacheFlat(unknownGroupHash, "!Unknown!", std::move(flatName), dbid);
            else
            {
                size_t idx = flatName.find('.');
                if (idx == std::string::npos)
                    cacheFlat(badGroupHash, "!BadName!", std::move(flatName), dbid);
                else
                {
                    // < 1 || > size = group as much as possible
                    for (int32_t i = 1; i != m_flatGroupNameDepth; ++i)
                    {
                        if (idx + 1 == flatName.size())
                            break;

                        const size_t idx2 = flatName.find('.', idx + 1);
                        if (idx2 == std::string::npos)
                            break;

                        idx = idx2;
                    }

                    const auto cGroupName = flatName.substr(0, idx);
                    cacheFlat(RED4ext::FNV1a64(cGroupName.c_str()), cGroupName.c_str(), flatName.c_str(), dbid);
                }
            }
        });

    if (const auto it = cachedFlatGroupsMap.find(unknownGroupHash); it != cachedFlatGroupsMap.cend())
    {
        auto& group = m_cachedFlatGroups[it.value()];
        group.m_name = fmt::format("{} - {} flats!", group.m_name, group.m_flats.size());
    }

    if (const auto it = cachedFlatGroupsMap.find(badGroupHash); it != cachedFlatGroupsMap.cend())
    {
        auto& group = m_cachedFlatGroups[it.value()];
        group.m_name = fmt::format("{} - {} flats!", group.m_name, group.m_flats.size());
    }

    std::ranges::sort(m_cachedFlatGroups, [](const CachedFlatGroup& acLeft, const CachedFlatGroup& acRight) { return SortString(acLeft.m_name, acRight.m_name); });
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
        if (sscanf(acString, "<TDBID:%X:%hhX", &dbid.name.hash, &dbid.name.length) != 2)
            (void)sscanf(acString, "ToTweakDBID{ hash = %X, length = %hhd", &dbid.name.hash, &dbid.name.length);
    }

    return dbid;
}

void TweakDBEditor::FilterRecords(bool aFilterTab, bool aFilterDropdown)
{
    const RED4ext::TweakDBID dbid = ExtractTweakDBIDFromString(s_recordsFilterBuffer);
    const RED4ext::TweakDBID dbidDropdown = ExtractTweakDBIDFromString(s_tweakdbidFilterBuffer);
    for (auto& group : m_cachedRecords)
    {
        bool anyRecordsVisible = false;
        std::for_each(
            std::execution::par_unseq, group.m_records.begin(), group.m_records.end(),
            [&](CachedRecord& record)
            {
                if (aFilterTab)
                {
                    bool isFiltered = false;
                    if (s_recordsFilterBuffer[0] != '\0' && record.m_dbid != dbid && !StringContains(record.m_name, s_recordsFilterBuffer, s_recordsFilterIsRegex))
                    {
                        isFiltered = true;

                        for (CachedFlat& flat : record.m_flats)
                        {
                            if (flat.m_dbid == dbid || StringContains(flat.m_name, s_recordsFilterBuffer, s_recordsFilterIsRegex))
                            {
                                isFiltered = false;
                                break;
                            }
                        }
                    }

                    if (isFiltered)
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
                    if (s_tweakdbidFilterBuffer[0] != '\0' && record.m_dbid != dbidDropdown && !StringContains(record.m_name, s_tweakdbidFilterBuffer))
                    {
                        record.m_isDropdownFiltered = true;
                    }
                    else
                    {
                        record.m_isDropdownFiltered = false;
                    }
                }
            });

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
        for (auto& group : m_cachedFlatGroups)
        {
            group.m_isFiltered = false;
            for (auto& flat : group.m_flats)
            {
                flat.m_isFiltered = false;
            }
        }
    }
    else
    {
        const RED4ext::TweakDBID dbid = ExtractTweakDBIDFromString(s_flatsFilterBuffer);
        for (auto& group : m_cachedFlatGroups)
        {
            bool anyFlatsVisible = false;
            for (auto& flat : group.m_flats)
            {
                if (flat.m_dbid != dbid && !StringContains(flat.m_name, s_flatsFilterBuffer, s_flatsFilterIsRegex))
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
    if (aWidth != 0.0f)
        ImGui::SetNextItemWidth(aWidth);
    const std::string recordName = GetTweakDBIDStringRecord(aDBID);
    const bool comboOpened = ImGui::BeginCombo(acpLabel, recordName.c_str(), ImGuiComboFlags_HeightLargest);
    if (ImGui::IsItemHovered())
    {
        RED4ext::Handle<RED4ext::IScriptable> record;
        if (RED4ext::TweakDB::Get()->TryGetRecord(aDBID, record))
        {
            const RED4ext::CName typeName = record->GetType()->GetName();
            ImGui::SetTooltipUnformatted(typeName.ToString());
        }
        else
        {
            ImGui::SetTooltipUnformatted("ERROR_RECORD_NOT_FOUND");
        }
    }
    if (comboOpened)
    {
        static float searchTimer = 0.0f;
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputTextWithHint("##dropdownSearch", "Search", s_tweakdbidFilterBuffer, sizeof(s_tweakdbidFilterBuffer)))
        {
            searchTimer = c_searchDelay;
        }

        if (searchTimer != 0.0f)
        {
            searchTimer -= ImGui::GetIO().DeltaTime;
            if (searchTimer <= 0.0f)
            {
                FilterRecords(false, true);
                searchTimer = 0.0f;
            }
        }

        if (ImGui::BeginChild("##dropdownScroll", ImVec2(0, g_comboDropdownHeight)))
        {
            for (const auto& recordGroup : m_cachedRecords)
            {
                for (const auto& record : recordGroup.m_records)
                {
                    if (record.m_isDropdownFiltered)
                        continue;

                    const bool isSelected = record.m_dbid == aDBID;
                    if (ImGui::NextItemVisible(ImVec2(1.0f, ImGui::GetTextLineHeight())) && ImGui::Selectable(record.m_name.c_str(), isSelected))
                    {
                        aDBID = record.m_dbid;
                        valueChanged = true;
                        ImGui::CloseCurrentPopup();
                        break;
                    }

                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltipUnformatted(recordGroup.m_name.c_str());

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
    return name;
}

bool TweakDBEditor::GetTweakDBIDStringRecord(RED4ext::TweakDBID aDBID, std::string& aString)
{
    aString = EngineTweaks::Get().GetVM().GetTDBIDString(aDBID);
    return !aString.starts_with('<') || !aString.ends_with('>');
}

std::string TweakDBEditor::GetTweakDBIDStringFlat(RED4ext::TweakDBID aDBID)
{
    std::string name;
    GetTweakDBIDStringFlat(aDBID, name);
    return name;
}

bool TweakDBEditor::GetTweakDBIDStringFlat(RED4ext::TweakDBID aDBID, std::string& aString)
{
    aString = EngineTweaks::Get().GetVM().GetTDBIDString(aDBID);
    return !aString.starts_with('<') || !aString.ends_with('>');
}

std::string TweakDBEditor::GetTweakDBIDStringQuery(RED4ext::TweakDBID aDBID)
{
    std::string name;
    GetTweakDBIDStringQuery(aDBID, name);
    return name;
}

bool TweakDBEditor::GetTweakDBIDStringQuery(RED4ext::TweakDBID aDBID, std::string& aString)
{
    aString = EngineTweaks::Get().GetVM().GetTDBIDString(aDBID);
    return aString[0] != '<' || aString[aString.size() - 1] != '>';
}

#pragma endregion

#pragma region Drawing flats

bool TweakDBEditor::DrawFlat(RED4ext::TweakDBID aDBID)
{
    RED4ext::CStackType data = TweakDB::InternalGetFlat(aDBID);

    if (!data.value)
    {
        ImGui::Text("'%s' is not found in TweakDB", GetTweakDBIDStringFlat(aDBID.value & 0xFFFFFFFFFF).c_str());
        return false;
    }

    ImGui::PushID(aDBID.name.hash);
    ImGui::PushID(aDBID.name.length);
    const bool isModified = DrawFlat(aDBID, data);
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
    if (aStackType.type == pTweakDBIDType)
        return DrawFlatTweakDBID(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pQuaternionType)
        return DrawFlatQuaternion(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pEulerAnglesType)
        return DrawFlatEulerAngles(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pVector3Type)
        return DrawFlatVector3(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pVector2Type)
        return DrawFlatVector2(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pColorType)
        return DrawFlatColor(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pGamedataLocKeyWrapperType)
        return DrawFlatLocKeyWrapper(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pRaRefCResourceType)
        return DrawFlatResourceAsyncRef(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pCNameType)
        return DrawFlatCName(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pBoolType)
        return DrawFlatBool(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pStringType)
        return DrawFlatString(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pFloatType)
        return DrawFlatFloat(aDBID, aStackType, aReadOnly);
    if (aStackType.type == pInt32Type)
        return DrawFlatInt32(aDBID, aStackType, aReadOnly);

    const auto typeName = aStackType.type->GetName();
    ImGui::Text("unsupported type: %s", typeName.ToString());
    return false;
}

// Needs a refactor
bool TweakDBEditor::DrawFlatArray(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly, bool aCollapsable)
{
    static TiltedPhoques::Map<uint64_t, RED4ext::ScriptInstance> editedArrays;

    auto* pArrayType = reinterpret_cast<RED4ext::CRTTIArrayType*>(aStackType.type);
    auto* pArrayInnerType = pArrayType->GetInnerType();
    const auto arrayTypeName = pArrayType->GetName();

    bool isModified = false;
    const bool isCachable = !aReadOnly && aDBID.IsValid();
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
                    auto* allocator = pArrayType->GetAllocator();
                    auto result = allocator->AllocAligned(pArrayType->GetSize(), pArrayType->GetAlignment());
                    pArrayType->Construct(result.memory);
                    pArrayType->Assign(result.memory, arrayInstance);
                    editedArrays.emplace(arrayKey, result.memory);
                }
            }
            else
            {
                ImGui::SameLine();
                if (ImGui::Button("cancel"))
                {
                    pArrayType->Destruct(arrayInstance);
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
                    const RED4ext::CStackType newStackType(aStackType.type, arrayInstance);
                    isModified = TweakDB::InternalSetFlat(aDBID, newStackType);

                    pArrayType->Destruct(arrayInstance);
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

        static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg;

        if (isCached)
        {
            // set background to red when the array is being edited
            ImGui::PushStyleColor(ImGuiCol_TableRowBg, ImVec4(1.00f, 0.00f, 0.00f, 0.06f));
            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(1.00f, 0.00f, 0.00f, 0.12f));
        }

        // TODO: fix column resizing issue on first frame
        if (ImGui::BeginTable("arrayElements", 2, tableFlags))
        {
            int32_t deleteElementIdx = -1;
            for (int32_t i = 0; i < static_cast<int32_t>(arraySize); ++i)
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
                const bool flatModified = DrawFlat({}, stackType, aReadOnly);
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
                pArrayInnerType->Construct(pArrayType->GetElement(arrayInstance, arraySize));
            }

            ImGui::EndTable();

            if (deleteElementIdx > -1)
            {
                pArrayType->RemoveAt(arrayInstance, static_cast<uint32_t>(deleteElementIdx));
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
    const auto* pDBID = static_cast<RED4ext::TweakDBID*>(aStackType.value);

    if (aReadOnly)
    {
        ImGui::SetNextItemWidth(-FLT_MIN);
        std::string recordName = GetTweakDBIDStringRecord(*pDBID);
        ImGui::InputText("", recordName.data(), recordName.size(), ImGuiInputTextFlags_ReadOnly);

        if (ImGui::IsItemHovered())
        {
            RED4ext::Handle<RED4ext::IScriptable> record;
            if (pDBID->IsValid() && RED4ext::TweakDB::Get()->TryGetRecord(pDBID->value, record))
            {
                const RED4ext::CName typeName = record->GetType()->GetName();
                ImGui::SetTooltipUnformatted(typeName.ToString());
            }
            else
            {
                ImGui::SetTooltipUnformatted("ERROR_RECORD_NOT_FOUND");
            }
        }

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputScalar("##raw", ImGuiDataType_U64, aStackType.value, nullptr, nullptr, "%016llX", ImGuiInputTextFlags_ReadOnly);
    }
    else
    {
        RED4ext::TweakDBID dbid = *pDBID;
        bool valueChanged = DrawRecordDropdown("", dbid, -FLT_MIN);

        constexpr int32_t flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal;
        ImGui::SetNextItemWidth(-FLT_MIN);
        valueChanged |= ImGui::InputScalar("##raw", ImGuiDataType_U64, &dbid.value, nullptr, nullptr, "%016llX", flags);

        if (valueChanged)
        {
            if (aDBID.IsValid())
            {
                const RED4ext::CStackType newStackType(aStackType.type, &dbid);
                return TweakDB::InternalSetFlat(aDBID, newStackType);
            }

            aStackType.type->Move(aStackType.value, &dbid);
            return true;
        }
    }

    return false;
}

bool TweakDBEditor::DrawFlatQuaternion(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pQuat = static_cast<RED4ext::Quaternion*>(aStackType.value);

    float i = pQuat->i;
    float j = pQuat->j;
    float k = pQuat->k;
    float r = pQuat->r;

    const int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::TextUnformatted("I");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool valueChanged = ImGui::InputFloat("##I", &i, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("J");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##J", &j, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("K");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##K", &k, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("R");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##R", &r, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Quaternion newQuat{};
        newQuat.i = i;
        newQuat.j = j;
        newQuat.k = k;
        newQuat.r = r;

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newQuat);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newQuat);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatEulerAngles(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pEular = static_cast<RED4ext::EulerAngles*>(aStackType.value);

    float roll = pEular->Roll;
    float pitch = pEular->Pitch;
    float yaw = pEular->Yaw;

    const int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::TextUnformatted("Roll ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool valueChanged = ImGui::InputFloat("##Roll", &roll, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("Pitch");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##Pitch", &pitch, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("Yaw  ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##Yaw", &yaw, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::EulerAngles newEular{};
        newEular.Roll = roll;
        newEular.Pitch = pitch;
        newEular.Yaw = yaw;

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newEular);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newEular);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatVector3(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pVec = static_cast<RED4ext::Vector3*>(aStackType.value);

    float x = pVec->X;
    float y = pVec->Y;
    float z = pVec->Z;

    const int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::TextUnformatted("X");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool valueChanged = ImGui::InputFloat("##X", &x, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("Y");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##Y", &y, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("Z");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##Z", &z, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Vector3 newVec{};
        newVec.X = x;
        newVec.Y = y;
        newVec.Z = z;

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newVec);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newVec);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatVector2(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pVec = static_cast<RED4ext::Vector2*>(aStackType.value);

    float x = pVec->X;
    float y = pVec->Y;

    const int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::TextUnformatted("X");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool valueChanged = ImGui::InputFloat("##X", &x, 0.0f, 0.0f, "%f", flags);

    ImGui::TextUnformatted("Y");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    valueChanged |= ImGui::InputFloat("##Y", &y, 0.0f, 0.0f, "%f", flags);

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Vector2 newVec{};
        newVec.X = x;
        newVec.Y = y;

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newVec);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newVec);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatColor(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pColor = static_cast<RED4ext::Color*>(aStackType.value);

    float rgba[4];
    rgba[0] = pColor->Red / 255.0f;
    rgba[1] = pColor->Green / 255.0f;
    rgba[2] = pColor->Blue / 255.0f;
    rgba[3] = pColor->Alpha / 255.0f;

    aReadOnly = true;
    ImGui::TextUnformatted("'Color' is not supported yet");
    ImGui::SameLine();

    const int32_t flags = aReadOnly ? ImGuiColorEditFlags_NoInputs : ImGuiColorEditFlags_None;
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool valueChanged = ImGui::ColorEdit4("", rgba, flags | ImGuiColorEditFlags_AlphaPreview);
    // Color picker returns true everytime it changes
    // It will overkill the FlatValuePool
    // Thankfully, I don't think 'Color' is used
    valueChanged = false;

    if (!aReadOnly && valueChanged)
    {
        RED4ext::Color newColor{};
        newColor.Red = static_cast<uint8_t>(std::clamp(rgba[0], 0.0f, 1.0f) * 255);
        newColor.Green = static_cast<uint8_t>(std::clamp(rgba[1], 0.0f, 1.0f) * 255);
        newColor.Blue = static_cast<uint8_t>(std::clamp(rgba[2], 0.0f, 1.0f) * 255);
        newColor.Alpha = static_cast<uint8_t>(std::clamp(rgba[3], 0.0f, 1.0f) * 255);

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newColor);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newColor);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatLocKeyWrapper(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pLocKey = static_cast<RED4ext::gamedataLocKeyWrapper*>(aStackType.value);

    ImGui::TextUnformatted("This is a LocalizationKey");
    ImGui::TextUnformatted("Game.GetLocalizedTextByKey(...)");

    uint64_t key = pLocKey->primaryKey;
    const int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool valueChanged = ImGui::InputScalar("", ImGuiDataType_U64, &key, nullptr, nullptr, nullptr, flags);

    {
        RED4ext::CString localizedText;
        ExecuteGlobalFunction("GetLocalizedTextByKey", &localizedText, key);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::TextUnformatted(localizedText.c_str(), localizedText.c_str() + localizedText.Length());
    }

    if (!aReadOnly && valueChanged)
    {
        RED4ext::gamedataLocKeyWrapper newLocKey;
        newLocKey.primaryKey = key;

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newLocKey);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newLocKey);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatResourceAsyncRef(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pRaRef = static_cast<RED4ext::ResourceAsyncReference<void>*>(aStackType.value);

    uint64_t hashRef = pRaRef->path.hash;

    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    flags |= ImGuiInputTextFlags_CharsHexadecimal;
    ImGui::SetNextItemWidth(-FLT_MIN);
    bool valueChanged = ImGui::InputScalar("", ImGuiDataType_U64, &hashRef, nullptr, nullptr, "%016llX", flags);

    if (ResourcesList::Get()->IsInitialized())
    {
        const std::string& resourceName = ResourcesList::Get()->Resolve(hashRef);

        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##resolvedHash", resourceName.c_str(), ImGuiComboFlags_HeightLargest))
        {
            static float searchTimer = -1.0f;
            static int resourcesCount = 0;
            static char comboSearchStr[256]{};
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::InputTextWithHint("##dropdownSearch", "Search", comboSearchStr, sizeof(comboSearchStr)))
            {
                searchTimer = c_searchDelay;
            }

            if (searchTimer != 0.0f)
            {
                searchTimer -= ImGui::GetIO().DeltaTime;
                if (searchTimer <= 0.0f)
                {
                    auto& resources = ResourcesList::Get()->GetResources();
                    resourcesCount = static_cast<int>(std::count_if(
                        std::execution::par_unseq, resources.begin(), resources.end(),
                        [](ResourcesList::Resource& resource)
                        {
                            if (comboSearchStr[0] == '\0' || StringContains(resource.m_name, comboSearchStr))
                            {
                                resource.m_isFiltered = false;
                            }
                            else
                            {
                                resource.m_isFiltered = true;
                            }

                            return !resource.m_isFiltered;
                        }));

                    searchTimer = 0.0f;
                }
            }

            if (ImGui::BeginChild("##dropdownScroll", ImVec2(0, g_comboDropdownHeight)))
            {
                const auto& resources = ResourcesList::Get()->GetResources();
                // DisplayStart/End is 'int'
                // Will this cause issues? current resources.size() is 1,485,150 million items
                assert(resources.size() < INT32_MAX);
                ImGuiListClipper clipper;
                clipper.Begin(resourcesCount, ImGui::GetTextLineHeightWithSpacing());
                while (clipper.Step())
                {
                    auto it = resources.begin();

                    // Skip 'DisplayStart' number of unfiltered items
                    for (int i = 0; i != clipper.DisplayStart; ++it)
                    {
                        const auto& resource = *it;
                        if (resource.m_isFiltered)
                            continue;

                        ++i;
                    }

                    for (int i = clipper.DisplayStart; i != clipper.DisplayEnd && it != resources.end(); ++it)
                    {
                        const auto& resource = *it;
                        if (resource.m_isFiltered)
                            continue;

                        const bool isSelected = resource.m_hash == hashRef;
                        if (ImGui::Selectable(resource.m_name.c_str(), isSelected))
                        {
                            hashRef = resource.m_hash;
                            valueChanged = true;
                            ImGui::CloseCurrentPopup();
                            break;
                        }

                        if (isSelected)
                            ImGui::SetItemDefaultFocus();

                        ++i;
                    }
                }
            }
            ImGui::EndChild();

            ImGui::EndCombo();
        }
    }

    if (!aReadOnly && valueChanged)
    {
        RED4ext::ResourceAsyncReference<void> newRaRef;
        newRaRef.path.hash = hashRef;

        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newRaRef);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newRaRef);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatCName(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pCName = static_cast<RED4ext::CName*>(aStackType.value);

    ImGui::TextUnformatted("Game is expecting specific values.");
    // Is it worth it to implement a dropdown like DrawTweakDBID?

    RED4ext::CName newCName;
    bool valueChanged = false;
    int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    flags |= ImGuiInputTextFlags_NoUndoRedo;

    auto* pStr = pCName->ToString();
    ImGui::SetNextItemWidth(-FLT_MIN);
    auto [strChanged, pModifiedStr] = ImGui::InputTextCStr("", pStr, strlen(pStr), flags);
    if (strChanged)
    {
        if (_stricmp(pModifiedStr->c_str(), "none") == 0)
            newCName.hash = 0;
        else
            newCName = RED4ext::CName(pModifiedStr->c_str());
        valueChanged = newCName.hash != pCName->hash;
    }

    if (ImGui::IsItemHovered() && _strnicmp(pStr, "LocKey#", 7) == 0)
    {
        RED4ext::CString localizedText;
        ExecuteGlobalFunction("GetLocalizedTextByKey", &localizedText, atoll(pStr + 7));
        ImGui::SetTooltipUnformatted(localizedText.c_str());
    }

    uint64_t hash = pCName->hash;
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool rawChanged = ImGui::InputScalar("##raw", ImGuiDataType_U64, &hash, nullptr, nullptr, "%016llX", flags | ImGuiInputTextFlags_CharsHexadecimal);
    if (rawChanged)
    {
        newCName.hash = hash;
        valueChanged = true;
    }

    if (valueChanged)
    {
        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newCName);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newCName);
        return true;
    }

    return valueChanged;
}

bool TweakDBEditor::DrawFlatBool(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pBool = static_cast<bool*>(aStackType.value);

    bool val = *pBool;
    const bool valueChanged = ImGui::Checkbox("", &val);
    if (!aReadOnly && valueChanged)
    {
        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &val);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &val);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatString(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pCString = static_cast<RED4ext::CString*>(aStackType.value);

    const int32_t flags = aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::SetNextItemWidth(-FLT_MIN);
    auto [valueChanged, pModifiedStr] = ImGui::InputTextCStr("", pCString->c_str(), pCString->Length(), flags);
    if (ImGui::IsItemHovered() && _strnicmp(pCString->c_str(), "LocKey#", 7) == 0)
    {
        RED4ext::CString localizedText;
        ExecuteGlobalFunction("GetLocalizedTextByKey", &localizedText, atoll(pCString->c_str() + 7));
        ImGui::SetTooltipUnformatted(localizedText.c_str());
    }

    if (valueChanged)
    {
        RED4ext::CString newCString(pModifiedStr->c_str());
        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &newCString);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &newCString);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatFloat(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pFloat = static_cast<float*>(aStackType.value);

    float val = *pFloat;
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool valueChanged = ImGui::InputFloat("", &val, 0, 0, "%f", aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue);
    if (valueChanged)
    {
        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &val);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &val);
        return true;
    }

    return false;
}

bool TweakDBEditor::DrawFlatInt32(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly)
{
    const auto* pInt = static_cast<int32_t*>(aStackType.value);

    int32_t val = *pInt;
    ImGui::SetNextItemWidth(-FLT_MIN);
    const bool valueChanged = ImGui::InputInt("", &val, 0, 0, aReadOnly ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_EnterReturnsTrue);
    if (valueChanged)
    {
        if (aDBID.IsValid())
        {
            const RED4ext::CStackType newStackType(aStackType.type, &val);
            return TweakDB::InternalSetFlat(aDBID, newStackType);
        }

        aStackType.type->Move(aStackType.value, &val);
        return true;
    }

    return false;
}

#pragma endregion

void TweakDBEditor::DrawRecordsTab()
{
    static float searchTimer = 0.0f;
    ImGui::SetNextItemWidth(
        -(ImGui::GetFrameHeight() + ImGui::CalcTextSize("Regex").x + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::GetStyle().FramePadding.x));
    if (ImGui::InputTextWithHint("##search", "Search", s_recordsFilterBuffer, sizeof(s_recordsFilterBuffer)))
    {
        searchTimer = c_searchDelay;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Regex", &s_recordsFilterIsRegex))
    {
        searchTimer = -1.0f;
    }

    if (searchTimer != 0.0f)
    {
        searchTimer -= ImGui::GetIO().DeltaTime;
        if (searchTimer <= 0.0f)
        {
            FilterRecords();
            searchTimer = 0.0f;
        }
    }

    if (!ImGui::BeginChild("##scrollable"))
        ImGui::EndChild();

    for (auto& group : m_cachedRecords)
    {
        if (group.m_isFiltered || !group.m_visibilityChecker.IsVisible())
            continue;

        group.m_visibilityChecker.Begin();

        if (ImGui::CollapsingHeader(group.m_name.c_str()))
        {
            group.Initialize();

            for (CachedRecord& record : group.m_records)
            {
                if (record.m_isFiltered || !record.m_visibilityChecker.IsVisible())
                    continue;

                record.m_visibilityChecker.Begin();

                if (ImGui::TreeNode(record.m_name.c_str()))
                {
                    static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg;

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
                            ImGui::SetNextItemWidth(-FLT_MIN);
                            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
                            ImGui::InputText("", flat.m_name.data(), flat.m_name.size(), ImGuiInputTextFlags_ReadOnly);
                            ImGui::PopStyleColor();
                            ImGui::PopID();

                            ImGui::TableNextColumn();
                            if (flat.m_isMissing)
                            {
                                ImGui::TextUnformatted("ERROR_FLAT_NOT_FOUND");
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

    std::shared_lock _(pTDB->mutex01);
    pTDB->queries.ForEach(
        [this](const RED4ext::TweakDBID& queryID, RED4ext::DynArray<RED4ext::TweakDBID>& recordIDs)
        {
            const auto queryName = GetTweakDBIDStringQuery(queryID.value);

            ImGui::PushID(queryID.name.hash);
            ImGui::PushID(queryID.name.length);
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
    static float searchTimer = 0.0f;
    ImGui::SetNextItemWidth(
        -(ImGui::GetFrameHeight() + ImGui::CalcTextSize("Regex").x + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::GetStyle().FramePadding.x));
    if (ImGui::InputTextWithHint("##search", "Search", s_flatsFilterBuffer, sizeof(s_flatsFilterBuffer)))
    {
        searchTimer = c_searchDelay;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("Regex", &s_flatsFilterIsRegex))
    {
        searchTimer = -1.0f;
    }

    if (searchTimer != 0.0f)
    {
        searchTimer -= ImGui::GetIO().DeltaTime;
        if (searchTimer <= 0.0f)
        {
            FilterFlats();
            searchTimer = 0.0f;
        }
    }

    if (!ImGui::BeginChild("##scrollable"))
        ImGui::EndChild();

    for (auto& group : m_cachedFlatGroups)
    {
        if (group.m_isFiltered || !group.m_visibilityChecker.IsVisible())
            continue;

        group.m_visibilityChecker.Begin();

        if (ImGui::CollapsingHeader(group.m_name.c_str()))
        {
            static constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg;

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
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32_BLACK_TRANS);
                    ImGui::InputText("", flat.m_name.data(), flat.m_name.size(), ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopStyleColor();
                    ImGui::PopID();

                    ImGui::TableNextColumn();
                    if (flat.m_isMissing)
                    {
                        ImGui::TextUnformatted("ERROR_FLAT_NOT_FOUND");
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

    if (ImGui::InputScalar("'Flats' Grouping depth", ImGuiDataType_S8, &m_flatGroupNameDepth, nullptr, nullptr, nullptr, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        RefreshFlats();
        FilterFlats();
    }

    ImGui::InputFloat("ComboBox dropdown height", &g_comboDropdownHeight, 0, 0);

    if (ImGui::Button("Refresh all"))
    {
        RefreshAll();
        FilterAll();
    }

    if (ImGui::BeginChild("CreateRecord", ImVec2(0.0f, 0.0f), true))
    {
        static auto status = "";
        static float statusTimer = 0.0f;
        static char recordName[256]{};
        static char comboSearchBuffer[70]{};
        static RED4ext::TweakDBID clonedRecordDBID;
        static RED4ext::CName recordTypeName;

        constexpr auto SetStatus = [](const char* acpStatus)
        {
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
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint("##dropdownSearch", "Search", comboSearchBuffer, sizeof(comboSearchBuffer));
            if (ImGui::BeginChild("##dropdownScroll", ImVec2(0, g_comboDropdownHeight)))
            {
                for (const auto& recordGroup : m_cachedRecords)
                {
                    if (comboSearchBuffer[0] != '\0' && !StringContains(recordGroup.m_name, comboSearchBuffer))
                    {
                        continue;
                    }

                    const bool isSelected = recordTypeName == recordGroup.m_typeName;
                    if (ImGui::NextItemVisible(ImVec2(1.0f, ImGui::GetTextLineHeight())) && ImGui::Selectable(recordGroup.m_name.c_str(), isSelected))
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

        if (statusTimer != 0.0f)
        {
            statusTimer -= ImGui::GetIO().DeltaTime;
            if (statusTimer <= 0.0f)
            {
                status = "";
                statusTimer = 0.0f;
            }
        }
        ImGui::TextUnformatted(status);
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
        std::shared_lock _(pTDB->mutex00);
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
    if (m_isInitialized)
        return;

    // order flats inside group
    std::ranges::sort(m_flats, [](const CachedFlat& aLeft, const CachedFlat& aRight) { return SortTweakDBIDString(aLeft.m_name, aRight.m_name); });

    m_isInitialized = true;
}

TweakDBEditor::CachedRecord::CachedRecord(std::string aName, RED4ext::TweakDBID aDBID) noexcept
    : m_name(std::move(aName))
    , m_dbid(aDBID)
{
}

void TweakDBEditor::CachedRecord::Initialize()
{
    if (m_isInitialized)
        return;

    InitializeFlats();

    std::ranges::sort(m_flats, [](const CachedFlat& aLeft, const CachedFlat& aRight) { return SortTweakDBIDString(aLeft.m_name, aRight.m_name); });

    m_isInitialized = true;
}

void TweakDBEditor::CachedRecord::InitializeFlats()
{
    if (!m_flats.empty())
        return;

    TiltedPhoques::Vector<uint64_t> recordFlats;
    EngineTweaks::Get().GetVM().GetTDBIDDerivedFrom(m_dbid.value, recordFlats);
    if (!recordFlats.empty())
    {
        m_flats.reserve(recordFlats.size());
        for (uint64_t flatID : recordFlats)
        {
            m_flats.emplace_back(GetTweakDBIDStringFlat(flatID), flatID);
        }
    }
}

void TweakDBEditor::CachedRecord::Update() const
{
    auto* pTDB = RED4ext::TweakDB::Get();
    pTDB->UpdateRecord(m_dbid);
}

TweakDBEditor::CachedRecordGroup::CachedRecordGroup(RED4ext::CName aTypeName)
    : m_name(aTypeName.ToString())
    , m_typeName(aTypeName)
{
}

void TweakDBEditor::CachedRecordGroup::Initialize()
{
    if (m_isInitialized)
        return;

    std::ranges::sort(m_records, [](const CachedRecord& aLeft, const CachedRecord& aRight) { return SortTweakDBIDString(aLeft.m_name, aRight.m_name); });

    m_isInitialized = true;
}

bool TweakDBEditor::ImGuiVisibilityChecker::IsVisible(bool aClaimSpaceIfInvisible) const
{
    return m_itemSize.y == 0 || ImGui::NextItemVisible(m_itemSize, aClaimSpaceIfInvisible);
}

void TweakDBEditor::ImGuiVisibilityChecker::Begin()
{
    m_beginCursorY = ImGui::GetCursorPosY();
}

void TweakDBEditor::ImGuiVisibilityChecker::End()
{
    const float endCursorY = ImGui::GetCursorPosY();
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

#endif
