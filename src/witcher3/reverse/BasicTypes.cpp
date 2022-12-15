#include <stdafx.h>

#include "BasicTypes.h"

#include "EngineTweaks.h"

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
