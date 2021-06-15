#pragma once

#include <RED4ext/REDfunc.hpp>
#include <RED4ext/DynArray.hpp>
#include <RED4ext/REDptr.hpp>
#include <RED4ext/REDhash.hpp>
#include <RED4ext/CName.hpp>
#include <RED4ext/CString.hpp>
#include <RED4ext/ISerializable.hpp>
#include <RED4ext/Types/InstanceType.hpp>
#include <RED4ext/Types/SimpleTypes.hpp>
#include <RED4ext/Types/CharacterCustomization.hpp>
#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/GameEngine.hpp>
#include <RED4ext/Scripting/Stack.hpp>
#include <RED4ext/Scripting/CProperty.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/OpcodeHandlers.hpp>
#include <RED4ext/MemoryAllocators.hpp>

#include "RED4ext/Types/generated/EulerAngles.hpp"
#include "RED4ext/Types/generated/Quaternion.hpp"
#include "RED4ext/Types/generated/Vector3.hpp"
#include "RED4ext/Types/generated/Vector4.hpp"

struct Vector3 : RED4ext::Vector3
{
    using base = RED4ext::Vector3;
    using base::base;

    Vector3(float aX = 0.f, float aY = 0.f, float aZ = 0.f)
        : base { aX, aY, aZ }
    {
    }
    
    [[nodiscard]] std::string ToString() const noexcept
    {
        return fmt::format("ToVector3{{ x = {0}, y = {1}, z = {2} }}", X, Y, Z);
    }

    bool operator==(const Vector3& acRhs) const noexcept
    {
        return X == acRhs.X && Y == acRhs.Y && Z == acRhs.Z;
    }
};

struct Vector4 : RED4ext::Vector4
{
    using base = RED4ext::Vector4;
    using base::base;

    Vector4(float aX = 0.f, float aY = 0.f, float aZ = 0.f, float aW = 0.f)
        : base { aX, aY, aZ, aW }
    {
    }

    [[nodiscard]] std::string ToString() const noexcept
    {
        return fmt::format("ToVector4{{ x = {0}, y = {1}, z = {2}, w = {3} }}", X, Y, Z, W);
    }

    bool operator==(const Vector4& acRhs) const noexcept
    {
        return X == acRhs.X && Y == acRhs.Y && Z == acRhs.Z && W == acRhs.W;
    }
};

struct EulerAngles : RED4ext::EulerAngles
{
    using base = RED4ext::EulerAngles;
    using base::base;

    EulerAngles(float aRoll = 0.f, float aPitch = 0.f, float aYaw = 0.f)
        : base { aRoll, aPitch, aYaw }
    {
    }
    
    [[nodiscard]] std::string ToString() const noexcept
    {
        return fmt::format("ToEulerAngles{{ roll = {0}, pitch = {1}, yaw = {2} }}", Roll, Pitch, Yaw);
    }

    bool operator==(const EulerAngles& acRhs) const noexcept
    {
        return Roll == acRhs.Roll && Pitch == acRhs.Pitch && Yaw == acRhs.Yaw;
    }
};

struct Quaternion : RED4ext::Quaternion
{
    using base = RED4ext::Quaternion;
    using base::base;

    Quaternion(float aI = 0.f, float aJ = 0.f, float aK = 0.f, float aR = 1.f)
        : base { aI, aJ, aK, aR }
    {
    }

    [[nodiscard]] std::string ToString() const noexcept
    {
        return fmt::format("ToQuaternion{{ i = {0}, j = {1}, k = {2}, r = {3} }}", i, j, k, r);
    }

    bool operator==(const Quaternion& acRhs) const noexcept
    {
        return i == acRhs.i && j == acRhs.j && k == acRhs.k && r == acRhs.r;
    }
};

struct CName : RED4ext::CName
{
    using base = RED4ext::CName;
    using base::base;

    static constexpr uint64_t HASH_HI_MASK = 0xFFFFFFFF00000000ull;
    static constexpr uint64_t HASH_LO_MASK = 0x00000000FFFFFFFFull;

    CName(uint64_t aHash = 0)
        : base(aHash)
    {
    }

    CName(uint32_t aHashLo, uint32_t aHashHi)
        : base((static_cast<uint64_t>(aHashHi) << 32) | aHashLo)
    {
    }

    CName(const std::string& aName)
        : base(aName.c_str())
    {
    }

    [[nodiscard]] std::string ToString() const noexcept
    {
        const auto resolved = base::ToString();
        if (!resolved)
            return fmt::format("ToCName{{ hash_lo = 0x{0:08X}, hash_hi = 0x{1:08X} }}", GetHashLo(), GetHashHi());
        return fmt::format("ToCName{{ hash_lo = 0x{0:08X}, hash_hi = 0x{1:08X} --[[ {2} --]] }}", GetHashLo(), GetHashHi(),
                           resolved);
    }

    void SetHashLo(uint32_t aHashLo)
    {
        hash = (hash & HASH_HI_MASK) | static_cast<uint64_t>(aHashLo);
    }

    void SetHashHi(uint32_t aHashHi)
    {
        hash = (static_cast<uint64_t>(aHashHi) << 32) | (hash & HASH_LO_MASK);
    }

    [[nodiscard]] uint32_t GetHashLo() const
    {
        return static_cast<uint32_t>(hash & HASH_LO_MASK);
    }

    [[nodiscard]] uint32_t GetHashHi() const
    {
        return static_cast<uint32_t>((hash & HASH_HI_MASK) >> 32);
    }
    
    bool operator==(const CName& acRhs) const noexcept
    {
        return base::operator==(acRhs);
    }
};

struct TweakDBID : RED4ext::TweakDBID
{
    using base = RED4ext::TweakDBID;
    using base::base;
    
    TweakDBID(RED4ext::TweakDBID other)
        : base { other }
    {
    }

    [[nodiscard]] std::string ToString() const noexcept
    {
        return fmt::format("ToTweakDBID{{ hash = 0x{0:08X}, length = {1:d} }}", nameHash, nameLength);
    }

    bool operator==(const TweakDBID& acRhs) const noexcept
    {
        return base::operator==(acRhs);
    }

    TweakDBID operator+(const std::string_view acName) const noexcept
    {
        return TweakDBID(*this, acName);
    }
};

struct ItemID : RED4ext::ItemID
{
    using base = RED4ext::ItemID;
    using base::base;

    ItemID(const TweakDBID& aId, uint32_t aRngSeed = 2, uint16_t aUnknown = 0, uint8_t aMaybeType = 0)
        : base { reinterpret_cast<const RED4ext::TweakDBID&>(aId), aRngSeed, aUnknown, aMaybeType, 0 }
    {
    }

    [[nodiscard]] std::string ToString() const noexcept
    {
        return fmt::format("ToItemID{{ id = {0}, rng_seed = {1}, unknown = {2}, maybe_type = {3} }}", TweakDBID(tdbid).ToString(),
                           rngSeed, unk0C, unk0E);
    }

    bool operator==(const ItemID& acRhs) const noexcept
    {
        return tdbid == acRhs.tdbid && rngSeed == acRhs.rngSeed;
    }
};

using Variant = RED4ext::Variant;
