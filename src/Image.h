#pragma once

#include "RED4ext/Api/v1/FileVer.hpp"
#include "RED4ext/Api/v1/SemVer.hpp"

struct Image
{
    void Initialize();

    static std::tuple<uint32_t, uint16_t> GetSupportedVersion() noexcept { return std::make_tuple(2, 50); }

    // 2.5 comes as two different exes depending on where you bought it, Steam/GOG and Battle.net, and the
    // Battle.net one was built later so its revision number is different, even though every address hash
    // CET hooks into is the same on both (checked all of Addresses.h against its cyberpunk2077_addresses.json)
    static bool IsVettedBuild(const RED4ext::v1::FileVer& acFileVersion) noexcept;

    uintptr_t base_address;
    mem::region TextRegion;
    RED4ext::v1::FileVer FileVersion{0, 0, 0, 0};
    RED4ext::v1::SemVer SemVersion{0, 0, 0, 0};
};
