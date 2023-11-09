#pragma once

union EnumValue
{
    int8_t int8;
    int16_t int16;
    int32_t int32;
    int64_t int64;
};

// Represents a specific value of an enum
// Always assumes enums may only be 1, 2, 4, or 8 bytes wide
struct Enum
{
    // Creates an Enum from its type description and a value's name
    Enum(const RED4ext::CEnum*, const std::string& acValue);
    // Creates an Enum from its type description and a value. If the provided value does not appear to be valid, 0 is used instead
    Enum(const RED4ext::CEnum*, int32_t aValue);
    // Creates an Enum from a stack value with type RED4ext::CEnum*. value is assumed to be an int64_t
    Enum(const RED4ext::CStackType& acStackType);
    // Creates an Enum from its type name and a value's name
    Enum(const std::string& acTypeName, const std::string& acValue);
    // Creates an Enum from its type name and a value. If the provided value does not appear to be valid, 0 is used instead
    Enum(const std::string& acTypeName, int32_t aValue);

    // Sets m_value based on acStackType's value
    void Get(const RED4ext::CStackType& acStackType) noexcept;
    // Sets acStackType's type and value to match this instance. Always allocates new memory for the value. Does not free memory if a value is already present
    void Set(RED4ext::CStackType& acStackType, TiltedPhoques::Allocator* apAllocator) const noexcept;
    // Sets acStackType's value based on m_value
    void Set(RED4ext::CStackType& acStackType) const noexcept;

    // Returns the name of the current value
    std::string GetValueName() const;

    // Sets value by name. If the provided name does not appear in the list of names, the value is not set
    void SetValueByName(const std::string& acValue);

    // Sets by value verified against enum list. If the provided value does not appear to be valid, the value is not set
    void SetValueSafe(int64_t aValue);

    std::string ToString() const;

    bool operator==(const Enum& acRhs) const noexcept;

    const RED4ext::CEnum* GetType() const;
    const void* GetValuePtr() const;
    // Returns m_value as an int64_t, casting with sign extension if needed
    int64_t GetValue() const;

protected:
    friend struct Scripting;

    const RED4ext::CEnum* m_cpType{nullptr};
    EnumValue m_value{0};
};
