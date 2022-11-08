#pragma once

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

    bool IsInitialized() const;

    const std::string& Resolve(uint64_t aHash);

    TiltedPhoques::Vector<Resource>& GetResources();

protected:
    void Reset();

private:
    std::atomic_bool m_isInitialized = false;
    TiltedPhoques::Vector<Resource> m_resources;
    TiltedPhoques::Map<uint64_t, Resource*> m_resourcesByHash;
};
