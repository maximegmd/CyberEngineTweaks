#pragma once

struct TweakDBMetadata
{
    inline static const std::string c_defaultFilename = "tweakdb.str";

    static TweakDBMetadata* Get();

    bool Initialize();

    bool IsInitialized() const;

    bool GetRecordName(RED4ext::TweakDBID aDBID, std::string& aName);

    bool GetFlatName(RED4ext::TweakDBID aDBID, std::string& aName);

    bool GetQueryName(RED4ext::TweakDBID aDBID, std::string& aName);

    bool HasREDModTweakDB() const;

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
