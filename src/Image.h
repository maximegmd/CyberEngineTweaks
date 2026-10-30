#pragma once

#include "RED4ext/Api/v1/FileVer.hpp"
#include "RED4ext/Api/v1/SemVer.hpp"

struct Image
{
    void Initialize();

    static std::tuple<uint32_t, uint16_t> GetSupportedVersion() noexcept { return std::make_tuple(2, 50); }

    uintptr_t base_address;
#ifndef __APPLE__
    mem::region TextRegion;
#endif
    RED4ext::v1::FileVer FileVersion{0, 0, 0, 0};
    RED4ext::v1::SemVer SemVersion{0, 0, 0, 0};
};
