#pragma once

struct Image
{
    void Initialize();

	static std::tuple<uint32_t, uint16_t> GetSupportedVersion() noexcept
	{
        return std::make_tuple(1, 12);
	}

    static uint64_t MakeVersion(uint32_t aMajor, uint16_t aMinor) noexcept
    {
        return static_cast<uint64_t>(aMajor) << 32 | static_cast<uint64_t>(aMinor) << 16;
    }

    std::tuple<uint32_t, uint16_t> GetVersion() const noexcept
    {
        return std::make_tuple(static_cast<uint32_t>(version >> 32), static_cast<uint16_t>((version >> 16) & 0xFFFF));
    }

    uint64_t version{0};
    uintptr_t base_address;
    mem::region TextRegion;
};
