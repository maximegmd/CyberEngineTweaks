#pragma once

struct Image
{
    void Initialize();

	static std::tuple<uint32_t, uint16_t> GetSupportedVersion() noexcept
	{
        return std::make_tuple(1, 60);
	}

    static uint64_t MakeVersion(const uint32_t acMajor, const uint16_t acMinor) noexcept
    {
        return static_cast<uint64_t>(acMajor) << 32 | static_cast<uint64_t>(acMinor) << 16;
    }

    [[nodiscard]] std::tuple<uint32_t, uint16_t> GetVersion() const noexcept
    {
        return std::make_tuple(static_cast<uint32_t>(version >> 32), static_cast<uint16_t>((version >> 16) & 0xFFFF));
    }

    uint64_t version{0};
    uintptr_t base_address;
    mem::region TextRegion;
};
