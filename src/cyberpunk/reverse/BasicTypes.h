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
    CName(uint64_t aHash = 0)
        : hash(aHash)
    {
    }

    CName(uint32_t aHashLo, uint32_t aHashHi)
        : hash_lo(aHashLo)
        , hash_hi(aHashHi)
    {
    }

    CName(const std::string& aName)
    {
        if (aName != "None")
            hash = RED4ext::FNV1a64(aName.c_str());
        else
            hash = 0;
    }

    union
    {
        uint64_t hash{0};
        struct
        {
            uint32_t hash_lo;
            uint32_t hash_hi;
        };
    };

    std::string AsString() const noexcept;
    std::string ToString() const noexcept;

    bool operator==(const CName& acRhs) const noexcept;

    static void Add(const std::string& aName);
};

#pragma pack(push, 1)
struct TweakDBID
{
    TweakDBID() = default;

    TweakDBID(uint32_t aNameHash, uint8_t aNameLength)
    {
        this->name_hash = aNameHash;
        this->name_length = aNameLength;
        this->unk5 = 0;
        this->unk7 = 0;
    }

    TweakDBID(uint64_t aValue) { this->value = aValue; }

    TweakDBID(const std::string_view aName)
    {
        name_hash = RED4ext::CRC32(aName.data(), 0);
        name_length = static_cast<uint8_t>(aName.size());
        unk5 = unk7 = 0;
    }

    TweakDBID(const TweakDBID& aBase, const std::string_view aName)
    {
        name_hash = RED4ext::CRC32(aName.data(), aBase.name_hash);
        name_length = static_cast<uint8_t>(aName.size() + aBase.name_length);
        unk5 = unk7 = 0;
    }

    std::string AsString() const noexcept;
    std::string ToString() const noexcept;

    bool operator==(const TweakDBID& acRhs) const noexcept;
    TweakDBID operator+(const std::string_view acName) const noexcept;

    union
    {
        uint64_t value{0};
        struct
        {
            uint32_t name_hash;
            uint8_t name_length;
            uint16_t unk5;
            uint8_t unk7;
        };
    };
};

static_assert(sizeof(TweakDBID) == 8);

struct ItemID
{
    ItemID() = default;
    ItemID(const TweakDBID& aId, uint32_t aRngSeed = 2, uint16_t aUnknown = 0, uint8_t aMaybeType = 0)
        : id(aId)
        , rng_seed(aRngSeed)
        , unknown(aUnknown)
        , maybe_type(aMaybeType)
        , pad(0)
    {
    }

    std::string ToString() const noexcept;

    bool operator==(const ItemID& acRhs) const noexcept;

    TweakDBID id;
    uint32_t rng_seed{2};
    uint16_t unknown{0};
    uint8_t maybe_type{0};
    uint8_t pad{0};
};

static_assert(sizeof(ItemID) == 0x10);

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
        kInlineSize = 16,
        kInlineAlignment = 8
    };

    enum : uintptr_t
    {
        kInlineFlag = 1,
        kTypeMask = ~kInlineFlag,
    };

    const RED4ext::CBaseRTTIType* type{nullptr};
    union
    {
        mutable uint8_t inlined[kInlineSize]{0};
        RED4ext::ScriptInstance instance;
    };
};

static_assert(sizeof(Variant) == 0x18);

struct CRUID
{
    CRUID(uint64_t aHash = 0)
        : hash(aHash)
    {
    }

    uint64_t hash;

    std::string ToString() const noexcept;

    bool operator==(const CRUID& acRhs) const noexcept;
};

static_assert(sizeof(CRUID) == 0x8);

struct gamedataLocKeyWrapper
{
    gamedataLocKeyWrapper(uint64_t aHash = 0)
        : hash(aHash)
    {
    }

    uint64_t hash;

    std::string ToString() const noexcept;

    bool operator==(const gamedataLocKeyWrapper& acRhs) const noexcept;
};

static_assert(sizeof(gamedataLocKeyWrapper) == 0x8);

struct alignas(8) ScriptGameInstance
{
    RED4ext::GameInstance* gameInstance;
    int8_t unk8;
    int64_t unk10;
};

static_assert(sizeof(ScriptGameInstance) == 0x18);

#pragma pack(pop)