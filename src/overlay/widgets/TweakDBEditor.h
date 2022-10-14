#pragma once

#include "Widget.h"

struct CDPRTweakDBMetadata
{
    inline static const std::string c_defaultFilename = "tweakdb.str";

    static CDPRTweakDBMetadata* Get();

    bool Initialize();

    bool IsInitialized();

    bool GetRecordName(RED4ext::TweakDBID aDBID, std::string& aName);

    bool GetFlatName(RED4ext::TweakDBID aDBID, std::string& aName);

    bool GetQueryName(RED4ext::TweakDBID aDBID, std::string& aName);

    bool HasREDModTweakDB();

protected:
    int32_t ReadCompressedInt(std::istream& aFile);

    void ReadTDBIDNameArray(std::istream& aFile, uint32_t aCount, TiltedPhoques::Map<uint64_t, std::string>& aOutMap);

    void Reset();

private:
    struct Header
    {
        uint32_t m_magic = 0;   // a hash of all types currently supported
        uint32_t m_version = 0; // 1
        uint32_t m_recordsCount = 0;
        uint32_t m_flatsCount = 0;
        uint32_t m_queriesCount = 0;
    };

    bool m_isInitialized = false;
    Header m_header;
    TiltedPhoques::Map<uint64_t, std::string> m_records;
    TiltedPhoques::Map<uint64_t, std::string> m_flats;
    TiltedPhoques::Map<uint64_t, std::string> m_queries;
};

using TOodleLZ_Decompress = size_t(*)(char *in, int insz, char *out, int outsz, int wantsFuzzSafety, int b, int c, void *d, void *e, void *f, void *g, void *workBuffer, size_t workBufferSize, int j);

struct ResourcesList
{
    inline static const std::string c_defaultFilename = "usedhashes.kark";

    struct Resource
    {
        bool m_isFiltered;
        std::string m_name;
        uint64_t m_hash;

        Resource(std::string aName) noexcept;

        Resource(Resource&&) noexcept = default;
        Resource& operator=(Resource&&) noexcept = default;
    };

    static ResourcesList* Get();

    bool Initialize();

    bool IsInitialized();

    const std::string& Resolve(uint64_t aHash);

    TiltedPhoques::Vector<Resource>& GetResources();

protected:
    void Reset();

private:
    bool m_isInitialized = false;
    TiltedPhoques::Vector<Resource> m_resources;
    TiltedPhoques::Map<uint64_t, Resource*> m_resourcesByHash;
};

struct LuaVM;

struct TweakDBEditor : Widget
{
    TweakDBEditor(LuaVM& aVm);
    ~TweakDBEditor() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;
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
    static bool DrawFlatQuaternion(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatEulerAngles(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatVector3(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatVector2(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatColor(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatLocKeyWrapper(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType,
                                      bool aReadOnly = false);
    static bool DrawFlatResourceAsyncRef(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatCName(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatBool(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatString(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatFloat(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);
    static bool DrawFlatInt32(RED4ext::TweakDBID aDBID, RED4ext::CStackType& aStackType, bool aReadOnly = false);

    void DrawRecordsTab();
    void DrawQueriesTab();
    void DrawFlatsTab();
    void DrawAdvancedTab();

private:
    // like ImGuiListClipper but supports dynamic size
    struct ImGuiVisibilityChecker
    {
        bool IsVisible(bool aClaimSpaceIfInvisible = true) const;
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
        bool m_isInitialized = false;
        std::string m_name;
        TiltedPhoques::Vector<CachedFlat> m_flats;
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
        bool m_isInitialized = false;
        std::string m_name;
        RED4ext::TweakDBID m_dbid;
        TiltedPhoques::Vector<CachedFlat> m_flats;
        ImGuiVisibilityChecker m_visibilityChecker;

        CachedRecord(std::string aName, RED4ext::TweakDBID aDBID) noexcept;
        CachedRecord(CachedRecord&&) noexcept = default;
        CachedRecord& operator=(CachedRecord&&) noexcept = default;
        void Initialize();
        void InitializeFlats();
        void Update() const;
    };

    struct CachedRecordGroup
    {
        bool m_isFiltered = false;
        bool m_isInitialized = false;
        std::string m_name;
        RED4ext::CName m_typeName;
        TiltedPhoques::Vector<CachedRecord> m_records;
        ImGuiVisibilityChecker m_visibilityChecker;

        CachedRecordGroup(RED4ext::CName aTypeName);
        CachedRecordGroup(CachedRecordGroup&&) noexcept = default;
        CachedRecordGroup& operator=(CachedRecordGroup&&) noexcept = default;
        void Initialize();
    };

    LuaVM& m_vm;
    bool m_initialized = false;
    int32_t m_flatGroupNameDepth = 1;
    TiltedPhoques::Vector<CachedFlatGroup> m_cachedFlatGroups;
    TiltedPhoques::Vector<CachedRecordGroup> m_cachedRecords;
    static bool s_recordsFilterIsRegex;
    static bool s_flatsFilterIsRegex;
    static char s_recordsFilterBuffer[256];
    static char s_flatsFilterBuffer[256];
    static char s_tweakdbidFilterBuffer[256];
};
