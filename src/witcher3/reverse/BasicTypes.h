#pragma once

struct Vector3
{
    Vector3(float aX = 0.f, float aY = 0.f, float aZ = 0.f)
        : x(aX)
        , y(aY)
        , z(aZ)
    {
    }

    float x;
    float y;
    float z;

    std::string ToString() const noexcept;

    bool operator==(const Vector3& acRhs) const noexcept;
};

struct Vector4
{
    Vector4(float aX = 0.f, float aY = 0.f, float aZ = 0.f, float aW = 0.f)
        : x(aX)
        , y(aY)
        , z(aZ)
        , w(aW)
    {
    }

    float x;
    float y;
    float z;
    float w;

    std::string ToString() const noexcept;

    bool operator==(const Vector4& acRhs) const noexcept;
};

struct EulerAngles
{
    EulerAngles(float aRoll = 0.f, float aPitch = 0.f, float aYaw = 0.f)
        : roll(aRoll)
        , pitch(aPitch)
        , yaw(aYaw)
    {
    }

    float roll;
    float pitch;
    float yaw;

    std::string ToString() const noexcept;

    bool operator==(const EulerAngles& acRhs) const noexcept;
};

struct Quaternion
{
    Quaternion(float aI = 0.f, float aJ = 0.f, float aK = 0.f, float aR = 0.f)
        : i(aI)
        , j(aJ)
        , k(aK)
        , r(aR)
    {
    }

    float i;
    float j;
    float k;
    float r;

    std::string ToString() const noexcept;

    bool operator==(const Quaternion& acRhs) const noexcept;
};