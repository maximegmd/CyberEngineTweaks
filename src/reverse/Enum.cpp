#include "stdafx.h"

#include "Enum.h"

Enum::Enum(const RED4ext::CStackType& aStackType)
{
    Get(aStackType);
}

Enum::Enum(const std::string& acTypeName, const std::string& acValue)
{
    const auto* cpType = RED4ext::CRTTISystem::Get()->GetEnum(RED4ext::FNV1a64(acTypeName.c_str()));
    if (cpType)
    {
        m_cpType = cpType;
        SetValueByName(acValue);
    }
}

Enum::Enum(const std::string& acTypeName, uint32_t aValue)
{
    const auto* cpType = RED4ext::CRTTISystem::Get()->GetEnum(RED4ext::FNV1a64(acTypeName.c_str()));
    if (cpType)
    {
        m_cpType = cpType;
        SetValueSafe(aValue);
    }
}

Enum::Enum(const RED4ext::CEnum* acpType, const std::string& acValue)
    : m_cpType(acpType)
{
    SetValueByName(acValue);
}

Enum::Enum(const RED4ext::CEnum* acpType, uint32_t aValue)
    : m_cpType(acpType)
{
    SetValueSafe(aValue);
}

void Enum::SetValueSafe(uint64_t aValue)
{
    for (uint32_t i = 0; i < m_cpType->valueList.size; ++i)
    {
        if (m_cpType->valueList[i] == static_cast<int64_t>(aValue))
        {
            m_value = aValue;
            break;
        }
    }
}

void Enum::Get(const RED4ext::CStackType& acStackType) noexcept
{
    m_cpType = static_cast<const RED4ext::CEnum*>(acStackType.type);
    switch (acStackType.type->GetSize())
    {
    case sizeof(uint8_t): m_value = *static_cast<uint8_t*>(acStackType.value); break;
    case sizeof(uint16_t): m_value = *static_cast<uint16_t*>(acStackType.value); break;
    case sizeof(uint32_t): m_value = *static_cast<uint32_t*>(acStackType.value); break;
    case sizeof(uint64_t): m_value = *static_cast<uint64_t*>(acStackType.value); break;
    }
}

void Enum::Set(RED4ext::CStackType& aStackType, TiltedPhoques::Allocator* apAllocator) const noexcept
{
    aStackType.type = const_cast<RED4ext::CEnum*>(m_cpType); // Sad cast
    switch (m_cpType->GetSize())
    {
    case sizeof(uint8_t): aStackType.value = apAllocator->New<uint8_t>(static_cast<uint8_t>(m_value)); break;
    case sizeof(uint16_t): aStackType.value = apAllocator->New<uint16_t>(static_cast<uint16_t>(m_value)); break;
    case sizeof(uint32_t): aStackType.value = apAllocator->New<uint32_t>(static_cast<uint32_t>(m_value)); break;
    case sizeof(uint64_t): aStackType.value = apAllocator->New<uint64_t>(static_cast<uint64_t>(m_value)); break;
    }
}

void Enum::Set(RED4ext::CStackType& acStackType) const noexcept
{
    switch (m_cpType->GetSize())
    {
    case sizeof(uint8_t): *static_cast<uint8_t*>(acStackType.value) = static_cast<uint8_t>(m_value); break;
    case sizeof(uint16_t): *static_cast<uint16_t*>(acStackType.value) = static_cast<uint16_t>(m_value); break;
    case sizeof(uint32_t): *static_cast<uint32_t*>(acStackType.value) = static_cast<uint32_t>(m_value); break;
    case sizeof(uint64_t): *static_cast<uint64_t*>(acStackType.value) = static_cast<uint64_t>(m_value); break;
    }
}

std::string Enum::GetValueName() const
{
    if (!m_cpType)
        return "";

    for (uint32_t i = 0; i < m_cpType->valueList.size; ++i)
    {
        if (m_cpType->valueList[i] == static_cast<int64_t>(m_value))
        {
            return m_cpType->hashList[i].ToString();
        }
    }

    return "";
}

void Enum::SetValueByName(const std::string& acValue)
{
    if (!m_cpType)
        return;

    const RED4ext::CName cValueName(acValue.c_str());

    for (uint32_t i = 0; i < m_cpType->hashList.size; ++i)
    {
        if (m_cpType->hashList[i] == cValueName)
        {
            m_value = m_cpType->valueList[i];
            break;
        }
    }
}

std::string Enum::ToString() const
{
    if (m_cpType)
    {
        const RED4ext::CName name = m_cpType->GetName();
        return name.ToString() + std::string(" : ") + GetValueName() + std::string(" (") + std::to_string(m_value) + std::string(")");
    }

    return "Invalid enum";
}

bool Enum::operator==(const Enum& acRhs) const noexcept
{
    if (!m_cpType || !acRhs.m_cpType)
        return false;

    const RED4ext::CName name = m_cpType->GetName();
    const RED4ext::CName nameRhs = acRhs.m_cpType->GetName();

    return name == nameRhs && m_value == acRhs.m_value;
}

const RED4ext::CEnum* Enum::GetType() const
{
    return m_cpType;
}

const void* Enum::GetValuePtr() const
{
    return &m_value;
}
