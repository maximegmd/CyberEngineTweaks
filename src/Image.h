#pragma once

#include "RED4ext/Api/v0/FileVer.hpp"
#include "RED4ext/Api/SemVer.hpp"


struct Image
{
    void Initialize();

    static std::tuple<uint32_t, uint16_t> GetSupportedVersion() noexcept { return std::make_tuple(2, 1000); }

    uintptr_t base_address;
    mem::region TextRegion;
    RED4ext::v0::FileVer FileVersion{0,0,0,0};
    RED4ext::v0::SemVer SemVersion{0, 0, 0, 0};
};
