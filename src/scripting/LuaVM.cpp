#include <stdafx.h>

#include "LuaVM.h"

#include <CET.h>
#include <Utils.h>

#include <reverse/TweakDB/ResourcesList.h>

const VKBind* LuaVM::GetBind(const VKModBind& acModBind) const
{
    return m_scripting.GetBind(acModBind);
}

const TiltedPhoques::Vector<VKBind>* LuaVM::GetBinds(const std::string& acModName) const
{
    return m_scripting.GetBinds(acModName);
}

const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& LuaVM::GetAllBinds() const
{
    return m_scripting.GetAllBinds();
}

bool LuaVM::ExecuteLua(const std::string& acCommand) const
{
    if (!m_initialized)
    {
        spdlog::get("scripting")->info("Command not executed! LuaVM is not yet initialized!");
        return false;
    }

    return m_scripting.ExecuteLua(acCommand);
}

void LuaVM::Update(float aDeltaTime)
{
    if (!m_initialized)
    {
        if (m_logCount.load(std::memory_order_relaxed) > 2)
            PostInitializeMods();

        return;
    }

    if (m_reload)
    {
        m_scripting.ReloadAllMods();

        m_reload = false;
    }

    m_scripting.TriggerOnUpdate(aDeltaTime);
}

void LuaVM::Draw() const
{
    if (!m_initialized || m_drawBlocked)
        return;

    m_scripting.TriggerOnDraw();
}

void LuaVM::ReloadAllMods()
{
    if (m_initialized)
        m_reload = true;
}

void LuaVM::OnOverlayOpen() const
{
    if (m_initialized)
        m_scripting.TriggerOnOverlayOpen();
}

void LuaVM::OnOverlayClose() const
{
    if (m_initialized)
        m_scripting.TriggerOnOverlayClose();
}

void LuaVM::Initialize()
{
    if (!IsInitialized())
        m_scripting.Initialize();
}

bool LuaVM::IsInitialized() const
{
    return m_initialized;
}

void LuaVM::BlockDraw(bool aBlockDraw)
{
    m_drawBlocked = aBlockDraw;
}

void LuaVM::RemoveTDBIDDerivedFrom(uint64_t aDBID)
{
    std::lock_guard _{ m_tdbidLock };

    const auto it = m_tdbidDerivedLookup.find(aDBID);
    if (it != m_tdbidDerivedLookup.end())
    {
        for (uint64_t flatID : it->second)
        {
            m_tdbidLookup.erase(flatID);
        }

        m_tdbidDerivedLookup.erase(it);
    }
}

bool LuaVM::GetTDBIDDerivedFrom(uint64_t aDBID, TiltedPhoques::Vector<uint64_t>& aDerivedList)
{
    std::shared_lock _{ m_tdbidLock };

    const auto it = m_tdbidDerivedLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidDerivedLookup.end())
        return false;

    aDerivedList.reserve(it->second.size());
    std::ranges::copy(it->second, std::back_inserter(aDerivedList));
    return true;
}

uint64_t LuaVM::GetTDBIDBase(uint64_t aDBID)
{
    std::shared_lock _{ m_tdbidLock };

    const auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return 0;
    return it->second.base;
}

TDBIDLookupEntry LuaVM::GetTDBIDLookupEntry(uint64_t aDBID)
{
    std::shared_lock _{ m_tdbidLock };

    const auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return { 0, "<unknown>" };

    return it->second;
}

std::string LuaVM::GetTDBDIDDebugString(TDBID aDBID) const
{
    RED4ext::TweakDBID internal(aDBID.value);
    return internal.HasTDBOffset()
        ? fmt::format("<TDBID:{:08X}:{:02X}:{:06X}>", internal.name.hash, internal.name.length, internal.ToTDBOffset())
        : fmt::format("<TDBID:{:08X}:{:02X}>", internal.name.hash, internal.name.length);
}

std::string LuaVM::GetTDBIDString(uint64_t aDBID, bool aOnlyRegistered)
{
    std::shared_lock _{ m_tdbidLock };

    auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return aOnlyRegistered ? "" : GetTDBDIDDebugString(TDBID{ aDBID });

    std::string string = it->second.name;
    uint64_t base = it->second.base;
    while (base != 0)
    {
        it = m_tdbidLookup.find(it->second.base);
        if (it == m_tdbidLookup.end())
        {
            string.insert(0, GetTDBDIDDebugString(TDBID{ base }));
            break;
        }
        string.insert(0, it->second.name);
        base = it->second.base;
    }

    return string;
}

void LuaVM::RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& acString)
{
    if (aValue == 0) return;
    std::lock_guard _{ m_tdbidLock };

    m_tdbidLookup[aValue] = { aBase, acString };
    if (aBase != 0)
        m_tdbidDerivedLookup[aBase].insert(aValue);
}

void LuaVM::PostInitializeScripting()
{
    m_scripting.PostInitializeScripting();
}

using TOodleLZ_Decompress = size_t(*)(char *in, int insz, char *out, int outsz, int wantsFuzzSafety, int b, int c, void *d, void *e, void *f, void *g, void *workBuffer, size_t workBufferSize, int j);

struct Header
{
    uint32_t m_magic = 0;   // a hash of all types currently supported
    uint32_t m_version = 0; // 1
    uint32_t m_recordsCount = 0;
    uint32_t m_flatsCount = 0;
    uint32_t m_queriesCount = 0;
};

int32_t ReadCompressedInt(std::istream& aFile)
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

void ReadTDBIDNameArray(std::istream& aFile, uint32_t aCount, TiltedPhoques::Map<uint64_t, TDBIDLookupEntry>& aOutMap)
{
    for (uint32_t i = 0; i < aCount; ++i)
    {
        const int32_t length = ReadCompressedInt(aFile);
        std::string str;
        str.resize(length);
        aFile.read(str.data(), length);
        aOutMap.try_emplace(TweakDBID(str).value, 0, std::move(str));
    }
}

bool InitializeTweakDBMetadata(TiltedPhoques::Map<uint64_t, TDBIDLookupEntry>& lookup)
{
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

    const auto tdbstrPathRAW = GetAbsolutePath(L"tweakdb.str", CET::Get().GetPaths().TweakDB(), false, true);
    if (!tdbstrPathRAW.empty())
    {
        try
        {
            std::ifstream file(tdbstrPathRAW, std::ios::binary);
            file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);
            Header header;
            file.read(reinterpret_cast<char*>(&header), sizeof(Header));
            assert(header.m_version == 1);
            ReadTDBIDNameArray(file, header.m_recordsCount, lookup);
            ReadTDBIDNameArray(file, header.m_flatsCount, lookup);
            ReadTDBIDNameArray(file, header.m_queriesCount, lookup);
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
        const auto tdbstrPath = GetAbsolutePath(L"tweakdbstr.kark", CET::Get().GetPaths().TweakDB(), false, true);
        if (tdbstrPath.empty())
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

            assert(size == decodedBytes.size());
            if (size != decodedBytes.size())
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

            Header header;
            tdbstrDecodedFile.read(reinterpret_cast<char*>(&header), sizeof(Header));
            assert(header.m_version == 1);
            ReadTDBIDNameArray(tdbstrDecodedFile, header.m_recordsCount, lookup);
            ReadTDBIDNameArray(tdbstrDecodedFile, header.m_flatsCount, lookup);
            ReadTDBIDNameArray(tdbstrDecodedFile, header.m_queriesCount, lookup);

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
    const auto moddedTweakDbFilePath = GetAbsolutePath(L"tweakdb.str", CET::Get().GetPaths().R6CacheModdedRoot(), false, true);
    if (moddedTweakDbFilePath.empty())
        return true;

    try
    {
        std::ifstream file(moddedTweakDbFilePath, std::ios::binary);
        file.exceptions(std::ios::badbit | std::ios::failbit | std::ios::eofbit);

        Header header;
        file.read(reinterpret_cast<char*>(&header), sizeof(Header));
        assert(header.m_version == 1);
        ReadTDBIDNameArray(file, header.m_recordsCount, lookup);
        ReadTDBIDNameArray(file, header.m_flatsCount, lookup);
        ReadTDBIDNameArray(file, header.m_queriesCount, lookup);
        Log::Info("CDPRTweakDBMetadata::Initalize() - REDMod TweakDB initialization successful!");
    }
    // this is easier for now
    // do not modify the return state since this isn't the main TweakDB
    catch (std::exception&)
    {
        Log::Warn("CDPRTweakDBMetadata::Initalize() - Failed to load REDMod TweakDB. Modded entries may not "
                  "be shown in the TweakDB Editor.");
    }

    return true;
}

void LuaVM::PostInitializeTweakDB()
{
    {
        std::lock_guard _{ m_tdbidLock };
        InitializeTweakDBMetadata(m_tdbidLookup);
    }

    ResourcesList::Get()->Initialize();

    m_scripting.PostInitializeTweakDB();
}

void LuaVM::PostInitializeMods()
{
    assert(!m_initialized);

    m_scripting.PostInitializeMods();

    spdlog::get("scripting")->info("LuaVM: initialization finished!");

    m_initialized = true;
}
