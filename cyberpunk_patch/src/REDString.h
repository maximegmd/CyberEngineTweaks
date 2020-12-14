#pragma once

#include <cstdint>

struct REDString
{
private:

	static constexpr uint64_t RecursiveHash(const char* acpStr, uint64_t aHash)
	{
		if (*acpStr) {
			aHash ^= *acpStr;
			aHash *= 1099511628211ull;

			return RecursiveHash(acpStr + 1, aHash);
		}

		return aHash;
	}

public:
	static constexpr uint64_t Hash(const char* acpStr)
	{
		return RecursiveHash(acpStr, 14695981039346656037ull);
	}
};