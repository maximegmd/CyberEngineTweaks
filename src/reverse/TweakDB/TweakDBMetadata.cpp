#include <stdafx.h>

#include "TweakDBMetadata.h"

#include <CET.h>
#include <Utils.h>

using TOodleLZ_Decompress = size_t(*)(char *in, int insz, char *out, int outsz, int wantsFuzzSafety, int b, int c, void *d, void *e, void *f, void *g, void *workBuffer, size_t workBufferSize, int j);

TweakDBMetadata* TweakDBMetadata::Get()
{
    static TweakDBMetadata instance;
    return &instance;
}

bool TweakDBMetadata::Initialize()
{
    Reset();

    // TODO - share decompression routine with ResourceList and simplify loading (quite a few samey branches)

    auto hOodleHandle = GetModuleHandle(TEXT("oo2ext_7_win64.dll"));
    if (hOodleHandle == nullptr)
    {
        Log::Error("CDPRTweakDBMetadata::Initalize() - Could not get Oodle access");
        return false;
    }

    auto OodleLZ_Decompress = reinterpret_cast<TOodleLZ_Decompress>(GetProcAddress(hOodleHandle, "OodleLZ_Decompress"));
    if (OodleLZ_Decompress == nullptr)
    {
        Log::Error("CDPRTweakDBMetadata::Initalize() - Could not get OodleLZ_Decompress");
        return false;
    }

    const auto tdbstrPathRAW = GetAbsolutePath(c_defaultFilenameRAW, CET::Get().GetPaths().TweakDB(), false, true);
    if (!tdbstrPathRAW .empty())
    {
        try
        {
            std::ifstream file(tdbstrPathRAW, std::ios::binary);
            file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
            file.read(reinterpret_cast<char*>(&m_header), sizeof(Header));
            assert(m_header.m_version == 1);
            ReadTDBIDNameArray(file, m_header.m_recordsCount, m_records);
            ReadTDBIDNameArray(file, m_header.m_flatsCount, m_flats);
            ReadTDBIDNameArray(file, m_header.m_queriesCount, m_queries);
            m_isInitialized = true;
        }
        // this is easier for now
        catch (std::exception&)
        {
            Log::Error("CDPRTweakDBMetadata::Initalize() - Failed to load tweakdb.str!");
            return false;
        }
    }
    else
    {
        const auto tdbstrPath = GetAbsolutePath(c_defaultFilename, CET::Get().GetPaths().TweakDB(), false, true);
        if (tdbstrPath .empty())
        {
            Log::Error("CDPRTweakDBMetadata::Initalize() - Missing tweakdbstr.kark!");
            return false;
        }

        try
        {
            constexpr size_t headerSize = 8;
            auto encodedBytes = ReadWholeBinaryFile(tdbstrPath);

            std::string decodedBytes;
            decodedBytes.resize(reinterpret_cast<uint32_t*>(encodedBytes.data())[1]);

            char workingMemory[0x80000];

            auto size = OodleLZ_Decompress(encodedBytes.data() + headerSize, static_cast<int>(encodedBytes.size() - headerSize), decodedBytes.data(),
                                           static_cast<int>(decodedBytes.size()), 1, 1, 0, nullptr, nullptr, nullptr, nullptr, workingMemory, std::size(workingMemory), 3);

            if (size != decodedBytes.size())
            {
                // this should not happen!
                assert(false);
                decodedBytes.resize(size);
            }
            else
            {
                Log::Error("CDPRTweakDBMetadata::Initalize() - Failed to decompress tweakdbstr.kark!");
                return false;
            }

            struct inline_buffer: std::streambuf {
                inline_buffer(char* base, std::ptrdiff_t n) {
                    this->setg(base, base, base + n);
                }
            } tdbstrDecodedBuffer(decodedBytes.data(), decodedBytes.size());
            std::istream tdbstrDecodedFile(&tdbstrDecodedBuffer);
            tdbstrDecodedFile.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

            tdbstrDecodedFile.read(reinterpret_cast<char*>(&m_header), sizeof(Header));
            assert(m_header.m_version == 1);
            ReadTDBIDNameArray(tdbstrDecodedFile, m_header.m_recordsCount, m_records);
            ReadTDBIDNameArray(tdbstrDecodedFile, m_header.m_flatsCount, m_flats);
            ReadTDBIDNameArray(tdbstrDecodedFile, m_header.m_queriesCount, m_queries);
            m_isInitialized = true;

            Log::Info("CDPRTweakDBMetadata::Initalize() - Primary TweakDB initialization successful!");
        }
        // this is easier for now
        catch (std::exception&)
        {
            Log::Error("CDPRTweakDBMetadata::Initalize() - Failed to load tweakdbstr.kark!");
            return false;
        }
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

bool TweakDBMetadata::IsInitialized() const
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

bool TweakDBMetadata::HasREDModTweakDB() const
{
    const auto filepath = CET::Get().GetPaths().R6CacheModdedRoot() / c_defaultFilename;
    return exists(filepath);
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
    for (uint32_t i = 0; i < aCount; ++i)
    {
        const int32_t length = ReadCompressedInt(aFile);
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
