#pragma once

#include "Widget.h"

struct LuaVM;

struct TweakDBEditor : Widget
{
    TweakDBEditor(LuaVM& aVm);
    ~TweakDBEditor() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;

protected:
    void RefreshAll();
    void RefreshRecords();
    void RefreshFlats();
    void FilterAll();
    void FilterRecords(bool aFilterTab = true, bool aFilterDropdown = false);
    void FilterFlats();
    bool DrawRecordDropdown(const char* acpLabel, RED4ext::TweakDBID& aDBID, float aWidth = 0);

    static std::string GetTweakDBIDStringRecord(RED4ext::TweakDBID aDBID);
    static bool GetTweakDBIDStringRecord(RED4ext::TweakDBID aDBID, std::string& aString);
    static std::string GetTweakDBIDStringFlat(RED4ext::TweakDBID aDBID);
    static bool GetTweakDBIDStringFlat(RED4ext::TweakDBID aDBID, std::string& aString);
    static std::string GetTweakDBIDStringQuery(RED4ext::TweakDBID aDBID);
    static bool GetTweakDBIDStringQuery(RED4ext::TweakDBID aDBID, std::string& aString);

    bool DrawFlat(RED4ext::TweakDBID aDBID);
    bool DrawFlat(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatArray(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false,
                       bool aCollapsable = true);
    bool DrawFlatTweakDBID(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatQuaternion(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatEulerAngles(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatVector3(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatVector2(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatColor(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatLocKeyWrapper(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatResourceAsyncRef(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatCName(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatBool(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatString(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatFloat(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    bool DrawFlatInt32(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);

    void DrawRecordsTab();
    void DrawQueriesTab();
    void DrawFlatsTab();
    void DrawAdvancedTab();

private:
    // like ImGuiListClipper but supports dynamic size
    struct ImGuiVisibilityChecker
    {
        bool IsVisible(bool aClaimSpaceIfInvisible = true);
        void Begin();
        void End();

    private:
        ImVec2 m_itemSize;
        float m_beginCursorY;
    };
    
    struct CachedFlat
    {
        bool m_isFiltered = false;
        bool m_isMissing = false;
        std::string m_name;
        RED4ext::TweakDBID m_dbid;
        ImGuiVisibilityChecker m_visibilityChecker;

        CachedFlat(std::string aName, RED4ext::TweakDBID aDBID) noexcept;
        CachedFlat(CachedFlat&&) noexcept = default;
        CachedFlat& operator=(CachedFlat&&) noexcept = default;
        void Update(int32_t aTDBOffset = -1);
    };

    struct CachedFlatGroup
    {
        bool m_isFiltered = false;
        bool m_initialized = false;
        std::string m_name;
        std::vector<CachedFlat> m_flats;
        ImGuiVisibilityChecker m_visibilityChecker;

        CachedFlatGroup(std::string aName) noexcept;
        CachedFlatGroup(CachedFlatGroup&&) noexcept = default;
        CachedFlatGroup& operator=(CachedFlatGroup&&) noexcept = default;
        void Initialize();
    };

    struct CachedRecord
    {
        bool m_isFiltered = false;
        bool m_isDropdownFiltered = false;
        bool m_initialized = false;
        std::string m_name;
        RED4ext::TweakDBID m_dbid;
        std::vector<CachedFlat> m_flats;
        ImGuiVisibilityChecker m_visibilityChecker;

        CachedRecord(std::string aName, RED4ext::TweakDBID aDBID) noexcept;
        CachedRecord(CachedRecord&&) noexcept = default;
        CachedRecord& operator=(CachedRecord&&) noexcept = default;
        void Initialize();
        void Update();
    };

    struct CachedRecordGroup
    {
        bool m_isFiltered = false;
        std::string m_name;
        RED4ext::CName m_typeName;
        std::vector<CachedRecord> m_records;
        ImGuiVisibilityChecker m_visibilityChecker;

        CachedRecordGroup(RED4ext::CName aTypeName);
        CachedRecordGroup(CachedRecordGroup&&) noexcept = default;
        CachedRecordGroup& operator=(CachedRecordGroup&&) noexcept = default;
    };

    LuaVM& m_vm;
    bool m_initialized = false;
    int32_t m_flatGroupNameDepth = 1;
    std::vector<CachedFlatGroup> m_cachedFlatGroups;
    std::vector<CachedRecordGroup> m_cachedRecords;
    static bool s_recordsFilterIsRegex;
    static bool s_flatsFilterIsRegex;
    static char s_recordsFilterBuffer[256];
    static char s_flatsFilterBuffer[256];
    static char s_tweakdbidFilterBuffer[256];
};
