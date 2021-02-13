#pragma once

struct Enum
{
    Enum(const RED4ext::CEnum*, const std::string& acValue);
    Enum(const RED4ext::CEnum*, uint32_t aValue);
    Enum(const RED4ext::CStackType& acStackType);
    Enum(const std::string& acTypeName, const std::string& acValue);
    Enum(const std::string& acTypeName, uint32_t aValue);

    void Get(const RED4ext::CStackType& acStackType) noexcept;
    void Set(RED4ext::CStackType& acStackType, TiltedPhoques::Allocator* apAllocator) const noexcept;
    void Set(RED4ext::CStackType& acStackType) const noexcept;

    // Returns the enum value by name
    std::string GetValueName() const;

    // Sets value by name in the enum list
    void SetValueByName(const std::string& acValue);

    // Sets by value verified against enum list
    void SetValueSafe(uint64_t aValue);

    std::string ToString() const;

    const RED4ext::CEnum* GetType() const;

protected:
    const RED4ext::CEnum*   m_cpType{ nullptr };
    uint64_t                m_value{ 0 };
};
