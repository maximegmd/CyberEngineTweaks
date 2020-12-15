#pragma once

#include <cstdint>

struct Image
{
	Image();

	static uint64_t MakeVersion(uint64_t aMajor, uint64_t aMinor)
	{
		return aMajor << 32 | aMinor << 16;
	}

	uint64_t version;
	uintptr_t base_address;
	uint8_t* pTextStart;
	uint8_t* pTextEnd;
};