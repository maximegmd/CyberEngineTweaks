#include <stdafx.h>

#include "TweakDBMetadata.h"

#include <CET.h>
#include <Utils.h>

TweakDBMetadata* TweakDBMetadata::Get()
{
    static TweakDBMetadata instance;
    return &instance;
}

bool TweakDBMetadata::Initialize()
{
    Reset();

    const auto tdbstrEncodedFilePath = GetAbsolutePath(c_defaultFilename + ".lz", CET::Get().GetPaths().TweakDB(), true, true);
    auto tdbstrDecodedBytes = DecodeFromLzma(tdbstrEncodedFilePath);
    if (tdbstrDecodedBytes.empty())
        return false;

    struct TDBStrDecodedBuffer: std::streambuf {
        TDBStrDecodedBuffer(char* base, std::ptrdiff_t n) {
            this->setg(base, base, base + n);
        }
    } tdbstrDecodedBuffer(reinterpret_cast<char*>(tdbstrDecodedBytes.data()), tdbstrDecodedBytes.size());
    std::istream tdbstrDecodedFile(&tdbstrDecodedBuffer);

    try
    {
        tdbstrDecodedFile.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
        tdbstrDecodedFile.read(reinterpret_cast<char*>(&m_header), sizeof(Header));
        assert(m_header.m_version == 1);
        ReadTDBIDNameArray(tdbstrDecodedFile, m_header.m_recordsCount, m_records);
        ReadTDBIDNameArray(tdbstrDecodedFile, m_header.m_flatsCount, m_flats);
        ReadTDBIDNameArray(tdbstrDecodedFile, m_header.m_queriesCount, m_queries);
        m_isInitialized = true;
    }
    // this is easier for now
    catch (std::exception&)
    {
        return false;
    }

    // check if we have a tweakdb that was changed by REDmod
    if (HasREDModTweakDB())
    {
        const auto moddedTweakDbFilePath = GetAbsolutePath(c_defaultFilename, CET::Get().GetPaths().R6CacheModdedRoot(), false, true);
        if (moddedTweakDbFilePath.empty())
            return true;

        try
        {
            std::ifstream file(moddedTweakDbFilePath, std::ios::binary);
            file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
            file.read(reinterpret_cast<char*>(&m_header), sizeof(Header));
            assert(m_header.m_version == 1);
            ReadTDBIDNameArray(file, m_header.m_recordsCount, m_records);
            ReadTDBIDNameArray(file, m_header.m_flatsCount, m_flats);
            ReadTDBIDNameArray(file, m_header.m_queriesCount, m_queries);
            Log::Info("CDPRTweakDBMetadata::Initalize() - REDMod TweakDB initialization successful!");
        }
        // this is easier for now
        // do not modify the return state since this isn't the main TweakDB
        catch (std::exception&)
        {
            Log::Warn("CDPRTweakDBMetadata::Initalize() - Failed to load REDMod TweakDB. Modded entries may not "
                      "be shown in the TweakDB Editor.");
        }
    }

    return true;
}

bool TweakDBMetadata::IsInitialized()
{
    return m_isInitialized;
}

bool TweakDBMetadata::GetRecordName(RED4ext::TweakDBID aDBID, std::string& aName)
{
    const auto it = m_records.find(aDBID.value & 0xFFFFFFFFFF);
    if (it == m_records.end())
        return false;

    aName = it->second;
    return true;
}

bool TweakDBMetadata::GetFlatName(RED4ext::TweakDBID aDBID, std::string& aName)
{
    const auto it = m_flats.find(aDBID.value & 0xFFFFFFFFFF);
    if (it == m_flats.end())
        return false;

    aName = it->second;
    return true;
}

bool TweakDBMetadata::GetQueryName(RED4ext::TweakDBID aDBID, std::string& aName)
{
    const auto it = m_queries.find(aDBID.value & 0xFFFFFFFFFF);
    if (it == m_queries.end())
        return false;

    aName = it->second;
    return true;
}

bool TweakDBMetadata::HasREDModTweakDB()
{
    auto filepath = CET::Get().GetPaths().R6CacheModdedRoot() / c_defaultFilename;
    return std::filesystem::exists(filepath);
}

int32_t TweakDBMetadata::ReadCompressedInt(std::istream& aFile)
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

void TweakDBMetadata::ReadTDBIDNameArray(std::istream& aFile, uint32_t aCount, TiltedPhoques::Map<uint64_t, std::string>& aOutMap)
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

void TweakDBMetadata::Reset()
{
    m_isInitialized = false;
    m_records.clear();
    m_flats.clear();
    m_queries.clear();
}
