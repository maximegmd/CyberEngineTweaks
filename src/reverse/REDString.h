#pragma once

#include <cstddef>
#include <cstdint>

struct REDString
{
    REDString();
    REDString(const char* acpData);
    REDString* Copy(REDString* other);
    void Destroy();

    char* ToString()
    {
        if (size >= 0x40000000u)
            return *(char**)str;
        return reinterpret_cast<char*>(str);
    }

    char str[0x14];
    uint32_t size{ 0x40000000 };
    uint8_t pad18[0x20 - 0x18];

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

static_assert(offsetof(REDString, size) == 0x14);
