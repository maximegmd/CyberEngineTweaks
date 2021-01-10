#include <stdafx.h>

#include "Pattern.h"

#include "Options.h"

bool CompareByteArray(uint8_t* Data, const std::vector<uint8_t>& aSignature)
{
    uint8_t* pData = Data;

    for (auto i : aSignature)
    {
        auto current = *pData;
        pData++;

        if (i == 0xCC)
        {
            continue;
        }

        if (current != i)
        {
            return false;
        }
    }

    return true;
}

uint8_t* FindSignature(uint8_t* apStart, uint8_t* apEnd, std::vector<uint8_t> aSignature) noexcept
{
    const uint8_t first = aSignature[0];
    const uint8_t* pEnd = apEnd - aSignature.size();

    for (; apStart < pEnd; ++apStart)
    {
        if (*apStart != first)
        {
            continue;
        }
        if (CompareByteArray(apStart, aSignature))
        {
            return apStart;
        }
    }

    return nullptr;
}

uint8_t* FindSignature(std::vector<uint8_t> aSignature) noexcept
{
    return FindSignature(Options::GameImage.pTextStart, Options::GameImage.pTextEnd, std::move(aSignature));
}
