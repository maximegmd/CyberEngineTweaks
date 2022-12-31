#include "stdafx.h"

#include "ResourcesList.h"

#include <CET.h>
#include <Utils.h>

using TOodleLZ_Decompress =
    size_t (*)(char* in, int insz, char* out, int outsz, int wantsFuzzSafety, int b, int c, void* d, void* e, void* f, void* g, void* workBuffer, size_t workBufferSize, int j);

ResourcesList::Resource::Resource(std::string aName) noexcept
    : m_isFiltered(false)
    , m_name(std::move(aName))
    , m_hash(RED4ext::FNV1a64(m_name.c_str()))
{
}

ResourcesList* ResourcesList::Get()
{
    static ResourcesList instance;
    return &instance;
}

bool ResourcesList::Initialize()
{
    Reset();

    // TODO - share decompression routine with TweakDBMetadata

    auto hOodleHandle = GetModuleHandle(TEXT("oo2ext_7_win64.dll"));
    if (hOodleHandle == nullptr)
    {
        spdlog::error("Could not get Oodle access");
        return false;
    }

    auto OodleLZ_Decompress = reinterpret_cast<TOodleLZ_Decompress>(GetProcAddress(hOodleHandle, "OodleLZ_Decompress"));
    if (OodleLZ_Decompress == nullptr)
    {
        spdlog::error("Could not get OodleLZ_Decompress");
        return false;
    }

    auto filepath = GetAbsolutePath(c_defaultFilename, CET::Get().GetPaths().TweakDB(), false, true);
    if (!exists(filepath))
        return false;

    m_resources.reserve(1485150);

    try
    {
        std::ifstream file(filepath, std::ios::binary);
        file.exceptions(std::ios::badbit);

        size_t headerSize = 8;

        std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
        std::string buffer;
        buffer.resize(*reinterpret_cast<uint32_t*>(content.data() + 4));

        char workingMemory[0x80000];
        auto size = OodleLZ_Decompress(
            content.data() + headerSize, static_cast<int>(content.size() - headerSize), buffer.data(), static_cast<int>(buffer.size()), 1, 1, 0, nullptr, nullptr, nullptr, nullptr,
            workingMemory, std::size(workingMemory), 3);

        assert(size == buffer.size());
        if (size != buffer.size())
        {
            spdlog::error("Decompress failed!");
            return false;
        }

        std::istringstream iss(buffer);

        std::string filename;
        while (std::getline(iss, filename))
        {
            filename.resize(filename.size() - 1);
            m_resources.emplace_back(filename);
        }

        for (auto& resource : m_resources)
        {
            m_resourcesByHash.emplace(resource.m_hash, &resource);
        }

        return m_isInitialized = true;
    }
    // this is easier for now
    catch (std::exception&)
    {
        return m_isInitialized = false;
    }
}

bool ResourcesList::IsInitialized() const
{
    return m_isInitialized;
}

const std::string& ResourcesList::Resolve(uint64_t aHash)
{
    static std::string defaultName = "ERROR_UNKNOWN_RESOURCE";

    const auto it = m_resourcesByHash.find(aHash);

    return it == m_resourcesByHash.end() ? defaultName : it->second->m_name;
}

TiltedPhoques::Vector<ResourcesList::Resource>& ResourcesList::GetResources()
{
    return m_resources;
}

void ResourcesList::Reset()
{
    m_isInitialized = false;
    m_resources.clear();
    m_resourcesByHash.clear();
}
