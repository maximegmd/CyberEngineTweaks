#pragma once

#include "LuaRED.h"

struct Enum
{
    Enum(const RED4ext::CEnum*, const std::string& value);
    Enum(const RED4ext::CEnum*, uint32_t value);
    Enum(const RED4ext::CStackType& stackType);
    Enum(const std::string& typeName, const std::string& value);
    Enum(const std::string& typeName, uint32_t value);

    void Get(const RED4ext::CStackType& stackType);
    void Set(RED4ext::CStackType& stackType, TiltedPhoques::Allocator* apAllocator);

    // Returns the enum value by name
    std::string GetValueName() const;

    // Sets value by name in the enum list
    void SetValueByName(const std::string& value);

    // Sets by value verified against enum list
    void SetValueSafe(uint64_t value);

    std::string ToString() const;

    const RED4ext::CEnum* GetType() const;

protected:
    const RED4ext::CEnum*	m_type{ nullptr };
    uint64_t				m_value{ 0 };
};
