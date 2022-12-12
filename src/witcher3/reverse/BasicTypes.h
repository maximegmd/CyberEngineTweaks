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

struct CName
{
    CName(uint32_t aHash = 0)
        : hash(aHash)
    {
    }

    CName(const std::string& aName)
    {
        if (aName != "None")
            hash = RED4ext::FNV1a32(aName.c_str());
        else
            hash = 0;
    }

    uint32_t m_hash;

    std::string AsString() const noexcept;
    std::string ToString() const noexcept;

    bool operator==(const CName& acRhs) const noexcept;

    static void Add(const std::string& aName);
};

struct Variant
{
    Variant() noexcept = default;
    Variant(const RED4ext::CBaseRTTIType* aType);
    Variant(const RED4ext::CBaseRTTIType* aType, const RED4ext::ScriptInstance aData);
    Variant(const RED4ext::CName& aTypeName, const RED4ext::ScriptInstance aData);
    Variant(const RED4ext::CStackType& aStack);
    Variant(const Variant& aOther);
    ~Variant();

    bool IsEmpty() const noexcept;
    bool IsInlined() const noexcept;

    RED4ext::CBaseRTTIType* GetType() const noexcept;
    RED4ext::ScriptInstance GetDataPtr() const noexcept;

    bool Init(const RED4ext::CBaseRTTIType* aType);
    bool Fill(const RED4ext::CBaseRTTIType* aType, const RED4ext::ScriptInstance aData);
    bool Extract(RED4ext::ScriptInstance aBuffer) const;
    void Free();

    inline static bool CanBeInlined(const RED4ext::CBaseRTTIType* aType) noexcept;

    enum
    {
        kInlineSize = 8,
        kInlineAlignment = 8
    };

    enum : uintptr_t
    {
        kInlineFlag = 1,
        kTypeMask = ~kInlineFlag,
    };

    CName name;
    const RED4ext::CBaseRTTIType* type{nullptr};
    union
    {
        mutable uint8_t inlined[kInlineSize]{0};
        void* instance;
    };
};

static_assert(sizeof(Variant) == 0x18);

#pragma pack(pop)