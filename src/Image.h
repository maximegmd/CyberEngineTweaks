#pragma once

#include <cstdint>
#include <tuple>

struct Image
{
	void Initialize();

	static uint64_t MakeVersion(uint64_t aMajor, uint64_t aMinor) noexcept
	{
		return aMajor << 32 | aMinor << 16;
	}

	std::tuple<uint32_t, uint32_t> GetVersion() const noexcept
	{
		return std::make_tuple(static_cast<uint32_t>(version >> 32), static_cast<uint32_t>((version >> 16) & 0xFFFF));
	}

	uint64_t version{0};
	uintptr_t base_address;
	uint8_t* pTextStart;
	uint8_t* pTextEnd;
};
