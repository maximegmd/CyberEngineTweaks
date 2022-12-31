#include "stdafx.h"

#include "BasicTypes.h"

#include <CET.h>

#include <spdlog/fmt/fmt.h>

std::string Vector3::ToString() const noexcept
{
    return fmt::format("ToVector3{{ x = {}, y = {}, z = {} }}", x, y, z);
}

bool Vector3::operator==(const Vector3& acRhs) const noexcept
{
    return x == acRhs.x && y == acRhs.y && z == acRhs.z;
}

std::string Vector4::ToString() const noexcept
{
    return fmt::format("ToVector4{{ x = {}, y = {}, z = {}, w = {} }}", x, y, z, w);
}

bool Vector4::operator==(const Vector4& acRhs) const noexcept
{
    return x == acRhs.x && y == acRhs.y && z == acRhs.z && w == acRhs.w;
}

std::string EulerAngles::ToString() const noexcept
{
    return fmt::format("ToEulerAngles{{ roll = {}, pitch = {}, yaw = {} }}", roll, pitch, yaw);
}

bool EulerAngles::operator==(const EulerAngles& acRhs) const noexcept
{
    return roll == acRhs.roll && pitch == acRhs.pitch && yaw == acRhs.yaw;
}

std::string Quaternion::ToString() const noexcept
{
    return fmt::format("ToQuaternion{{ i = {}, j = {}, k = {}, r = {} }}", i, j, k, r);
}

bool Quaternion::operator==(const Quaternion& acRhs) const noexcept
{
    return i == acRhs.i && j == acRhs.j && k == acRhs.k && r == acRhs.r;
}

std::string CName::AsString() const noexcept
{
    return RED4ext::CName(hash).ToString();
}

std::string CName::ToString() const noexcept
{
    const RED4ext::CName internal(hash);

    const auto resolved = internal.ToString();
    if (!resolved)
        return fmt::format("ToCName{{ hash_lo = 0x{:08X}, hash_hi = 0x{:08X} }}", hash_lo, hash_hi);
    return fmt::format("ToCName{{ hash_lo = 0x{:08X}, hash_hi = 0x{:08X} --[[ {} --]] }}", hash_lo, hash_hi, resolved);
}

bool CName::operator==(const CName& acRhs) const noexcept
{
    return hash == acRhs.hash;
}

void CName::Add(const std::string& aName)
{
    RED4ext::CNamePool::Add(aName.c_str());
}

std::string TweakDBID::AsString() const noexcept
{
    return CET::Get().GetVM().GetTDBIDString(value);
}

std::string TweakDBID::ToString() const noexcept
{
    const auto resolved = CET::Get().GetVM().GetTDBIDString(value, true);
    if (!resolved.empty())
        return fmt::format("ToTweakDBID{{ hash = 0x{:08X}, length = {:d} }}", name_hash, name_length);
    return fmt::format("ToTweakDBID{{ hash = 0x{:08X}, length = {:d} --[[ {} --]] }}", name_hash, name_length, resolved);
}

bool TweakDBID::operator==(const TweakDBID& acRhs) const noexcept
{
    return name_hash == acRhs.name_hash && name_length == acRhs.name_length;
}

TweakDBID TweakDBID::operator+(const std::string_view acName) const noexcept
{
    return {*this, acName};
}

std::string ItemID::ToString() const noexcept
{
    return fmt::format("ToItemID{{ id = {}, rng_seed = {}, unknown = {}, maybe_type = {} }}", id.ToString(), rng_seed, unknown, maybe_type);
}

bool ItemID::operator==(const ItemID& acRhs) const noexcept
{
    return id == acRhs.id && rng_seed == acRhs.rng_seed;
}

Variant::Variant(const RED4ext::CBaseRTTIType* aType)
    : Variant()
{
    if (aType)
    {
        Init(aType);
    }
}

Variant::Variant(const RED4ext::CBaseRTTIType* aType, const RED4ext::ScriptInstance aData)
    : Variant()
{
    if (aType)
    {
        Fill(aType, aData);
    }
}

Variant::Variant(const RED4ext::CName& aTypeName, const RED4ext::ScriptInstance aData)
    : Variant(RED4ext::CRTTISystem::Get()->GetType(aTypeName), aData)
{
}

Variant::Variant(const RED4ext::CStackType& aStack)
    : Variant(aStack.type, aStack.value)
{
}

Variant::Variant(const Variant& aOther)
    : Variant(aOther.GetType(), aOther.GetDataPtr())
{
}

Variant::~Variant()
{
    Free();
}

bool Variant::IsEmpty() const noexcept
{
    return !type;
}

bool Variant::IsInlined() const noexcept
{
    return reinterpret_cast<uintptr_t>(type) & kInlineFlag;
}

RED4ext::CBaseRTTIType* Variant::GetType() const noexcept
{
    return reinterpret_cast<RED4ext::CBaseRTTIType*>(reinterpret_cast<uintptr_t>(type) & kTypeMask);
}

RED4ext::ScriptInstance Variant::GetDataPtr() const noexcept
{
    return IsInlined() ? inlined : instance;
}

bool Variant::Init(const RED4ext::CBaseRTTIType* aType)
{
    if (!aType)
    {
        Free();
        return false;
    }

    const RED4ext::CBaseRTTIType* ownType = GetType();
    RED4ext::ScriptInstance ownData = GetDataPtr();

    if (ownType)
    {
        if (aType == ownType)
            return true;

        if (ownData)
        {
            ownType->Destruct(ownData);

            if (!IsInlined())
                ownType->GetAllocator()->Free(ownData);
        }
    }

    type = aType;

    if (CanBeInlined(aType))
    {
        type = reinterpret_cast<const RED4ext::CBaseRTTIType*>(reinterpret_cast<uintptr_t>(type) | kInlineFlag);
        ownData = inlined;
    }
    else
    {
        instance = aType->GetAllocator()->AllocAligned(aType->GetSize(), aType->GetAlignment()).memory;
        ownData = instance;
    }

    aType->Construct(ownData);

    return true;
}

bool Variant::Fill(const RED4ext::CBaseRTTIType* aType, const RED4ext::ScriptInstance aData)
{
    if (!Init(aType))
        return false;

    if (!aData)
        return false;

    GetType()->Assign(GetDataPtr(), aData);

    return true;
}

bool Variant::Extract(RED4ext::ScriptInstance aBuffer) const
{
    if (IsEmpty())
        return false;

    GetType()->Assign(aBuffer, GetDataPtr());

    return true;
}

void Variant::Free()
{
    if (IsEmpty())
        return;

    const RED4ext::CBaseRTTIType* ownType = GetType();
    const RED4ext::ScriptInstance ownData = GetDataPtr();

    if (ownData)
    {
        ownType->Destruct(ownData);

        if (!IsInlined())
            ownType->GetAllocator()->Free(ownData);
    }

    instance = nullptr;
    type = nullptr;
}

bool Variant::CanBeInlined(const RED4ext::CBaseRTTIType* aType) noexcept
{
    return aType->GetSize() <= kInlineSize && aType->GetAlignment() <= kInlineAlignment;
}

std::string CRUID::ToString() const noexcept
{
    return fmt::format("CRUID({}ull)", hash);
}

bool CRUID::operator==(const CRUID& acRhs) const noexcept
{
    return hash == acRhs.hash;
}

std::string gamedataLocKeyWrapper::ToString() const noexcept
{
    return fmt::format("LocKey({}ull)", hash);
}

bool gamedataLocKeyWrapper::operator==(const gamedataLocKeyWrapper& acRhs) const noexcept
{
    return hash == acRhs.hash;
}
