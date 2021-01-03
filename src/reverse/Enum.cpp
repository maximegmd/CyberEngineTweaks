#include <stdafx.h>

#include "Enum.h"

Enum::Enum(const RED4ext::CStackType& stackType)
{
    Get(stackType);
}

Enum::Enum(const std::string& typeName, const std::string& value)
{
    auto* pType = static_cast<RED4ext::CEnum*>(RED4ext::CRTTISystem::Get()->GetEnum(RED4ext::FNV1a(typeName.c_str())));
    if (pType)
    {
        m_type = pType;
        SetValueByName(value);
    }
}

Enum::Enum(const std::string& typeName, uint32_t value)
{
    auto* pType = static_cast<RED4ext::CEnum*>(RED4ext::CRTTISystem::Get()->GetEnum(RED4ext::FNV1a(typeName.c_str())));
    if (pType)
    {
        m_type = pType;
        SetValueSafe(static_cast<uint32_t>(value));
    }
}


Enum::Enum(const RED4ext::CEnum* pType, const std::string& value)
    : m_type(pType)
{
    SetValueByName(value);
}


Enum::Enum(const RED4ext::CEnum* pType, uint32_t value)
    : m_type(pType)
{
    SetValueSafe(static_cast<uint32_t>(value));
}

void Enum::SetValueSafe(uint64_t value)
{
    for (auto i = 0; i < m_type->valueList.size; ++i)
    {
        if (m_type->valueList[i] == value)
        {
            m_value = value;
            break;
        }
    }
}

void Enum::Get(const RED4ext::CStackType& stackType)
{
    m_type = static_cast<RED4ext::CEnum*>(stackType.type);
    switch (stackType.type->GetSize())
    {
        case sizeof(uint8_t) :
            m_value = *static_cast<uint8_t*>(stackType.value);
            break;
        case sizeof(uint16_t):
            m_value = *static_cast<uint16_t*>(stackType.value);
            break;
        case sizeof(uint32_t):
            m_value = *static_cast<uint32_t*>(stackType.value);
            break;
        case sizeof(uint64_t):
            m_value = *static_cast<uint64_t*>(stackType.value);
            break;
    }
}

void Enum::Set(RED4ext::CStackType& stackType, TiltedPhoques::Allocator* apAllocator)
{
    stackType.type = const_cast<RED4ext::CEnum*>(m_type); // Sad cast
    switch (m_type->GetSize())
    {
        case sizeof(uint8_t):
            stackType.value = apAllocator->New<uint8_t>(static_cast<uint8_t>(m_value));
            break;
        case sizeof(uint16_t):
            stackType.value = apAllocator->New<uint16_t>(static_cast<uint16_t>(m_value));
            break;
        case sizeof(uint32_t):
            stackType.value = apAllocator->New<uint32_t>(static_cast<uint32_t>(m_value));
            break;
        case sizeof(uint64_t):
            stackType.value = apAllocator->New<uint64_t>(static_cast<uint64_t>(m_value));
            break;
    }
}


std::string Enum::GetValueName() const
{
    for (auto i = 0; i < m_type->valueList.size; ++i)
    {
        if (m_type->valueList[i] == m_value)
        {
            return RED4ext::CName(m_type->hashList[i]).ToString();
        }
    }

    return "";
}


void Enum::SetValueByName(const std::string& value)
{
    for (auto i = 0; i < m_type->hashList.size; ++i)
    {
        if (m_type->hashList[i] == RED4ext::FNV1a(value.c_str()))
        {
            m_value = m_type->valueList[i];
            break;
        }
    }
}

std::string Enum::ToString() const
{
    if (m_type)
    {
        RED4ext::CName name;
        m_type->GetName(name);
        return name.ToString() + std::string(" : ") + GetValueName() + std::string(" (") + std::to_string(m_value) + std::string(")");
    }

    return "Invalid enum";
}

const RED4ext::CEnum* Enum::GetType() const
{
    return m_type;
}

