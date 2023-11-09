#include <stdafx.h>

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

Enum::Enum(const std::string& acTypeName, int32_t aValue)
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

Enum::Enum(const RED4ext::CEnum* acpType, int32_t aValue)
    : m_cpType(acpType)
{
    SetValueSafe(aValue);
}

void Enum::SetValueSafe(int64_t aValue)
{
    for (uint32_t i = 0; i < m_cpType->valueList.size; ++i)
    {
        int64_t val = m_cpType->valueList[i];
        switch (m_cpType->GetSize())
        {
        case sizeof(int8_t):
            if (static_cast<int8_t>(val) == static_cast<int8_t>(aValue))
            {
                m_value.int8 = static_cast<int8_t>(aValue);
            }
            break;
        case sizeof(int16_t):
            if (static_cast<int16_t>(val) == static_cast<int16_t>(aValue))
            {
                m_value.int16 = static_cast<int16_t>(aValue);
            }
            break;
        case sizeof(int32_t):
            if (static_cast<int32_t>(val) == static_cast<int32_t>(aValue))
            {
                m_value.int32 = static_cast<int32_t>(aValue);
            }
            break;
        case sizeof(int64_t):
            if (val == aValue)
            {
                m_value.int64 = aValue;
            }
            break;
        }
    }
}

void Enum::Get(const RED4ext::CStackType& acStackType) noexcept
{
    m_cpType = static_cast<const RED4ext::CEnum*>(acStackType.type);
    switch (acStackType.type->GetSize())
    {
    case sizeof(int8_t): m_value.int8 = *static_cast<int8_t*>(acStackType.value); break;
    case sizeof(int16_t): m_value.int16 = *static_cast<int16_t*>(acStackType.value); break;
    case sizeof(int32_t): m_value.int32 = *static_cast<int32_t*>(acStackType.value); break;
    case sizeof(int64_t): m_value.int64 = *static_cast<int64_t*>(acStackType.value); break;
    }
}

void Enum::Set(RED4ext::CStackType& aStackType, TiltedPhoques::Allocator* apAllocator) const noexcept
{
    aStackType.type = const_cast<RED4ext::CEnum*>(m_cpType); // Sad cast
    switch (m_cpType->GetSize())
    {
    case sizeof(int8_t): aStackType.value = apAllocator->New<int8_t>(m_value.int8); break;
    case sizeof(int16_t): aStackType.value = apAllocator->New<int16_t>(m_value.int16); break;
    case sizeof(int32_t): aStackType.value = apAllocator->New<int32_t>(m_value.int32); break;
    case sizeof(int64_t): aStackType.value = apAllocator->New<int64_t>(m_value.int64); break;
    }
}

void Enum::Set(RED4ext::CStackType& acStackType) const noexcept
{
    switch (m_cpType->GetSize())
    {
    case sizeof(int8_t): *static_cast<int8_t*>(acStackType.value) = m_value.int8; break;
    case sizeof(int16_t): *static_cast<int16_t*>(acStackType.value) = m_value.int16; break;
    case sizeof(int32_t): *static_cast<int32_t*>(acStackType.value) = m_value.int32; break;
    case sizeof(int64_t): *static_cast<int64_t*>(acStackType.value) = m_value.int64; break;
    }
}

std::string Enum::GetValueName() const
{
    if (!m_cpType)
        return "";
    int64_t val = GetValue();

    for (uint32_t i = 0; i < m_cpType->valueList.size; ++i)
    {
        if (m_cpType->valueList[i] == val)
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
            int64_t val = m_cpType->valueList[i];
            switch (m_cpType->GetSize())
            {
            case sizeof(int8_t): m_value.int8 = static_cast<int8_t>(val); break;
            case sizeof(int16_t): m_value.int16 = static_cast<int16_t>(val); break;
            case sizeof(int32_t): m_value.int32 = static_cast<int32_t>(val); break;
            case sizeof(int64_t): m_value.int64 = static_cast<int64_t>(val); break;
            }
            break;
        }
    }
}

std::string Enum::ToString() const
{
    if (m_cpType)
    {
        const RED4ext::CName name = m_cpType->GetName();
        std::string numStr;
        switch (m_cpType->GetSize())
        {
        case sizeof(int8_t): numStr = std::to_string(m_value.int8); break;
        case sizeof(int16_t): numStr = std::to_string(m_value.int16); break;
        case sizeof(int32_t): numStr = std::to_string(m_value.int32); break;
        case sizeof(int64_t): numStr = std::to_string(m_value.int64); break;
        default: return "Invalid enum";
        }
        return name.ToString() + std::string(" : ") + GetValueName() + std::string(" (") + numStr + std::string(")");
    }

    return "Invalid enum";
}

int64_t Enum::GetValue() const
{
    switch (m_cpType->GetSize())
    {
    case sizeof(int8_t): return static_cast<int64_t>(m_value.int8);
    case sizeof(int16_t): return static_cast<int64_t>(m_value.int16);
    case sizeof(int32_t): return static_cast<int64_t>(m_value.int32);
    case sizeof(int64_t):
    default: return m_value.int64;
    }
}

bool Enum::operator==(const Enum& acRhs) const noexcept
{
    if (!m_cpType || !acRhs.m_cpType)
        return false;

    const RED4ext::CName name = m_cpType->GetName();
    const RED4ext::CName nameRhs = acRhs.m_cpType->GetName();

    bool valuesEqual;
    switch (m_cpType->GetSize())
    {
    case sizeof(int8_t): valuesEqual = m_value.int8 == acRhs.m_value.int8; break;
    case sizeof(int16_t): valuesEqual = m_value.int16 == acRhs.m_value.int16; break;
    case sizeof(int32_t): valuesEqual = m_value.int32 == acRhs.m_value.int32; break;
    case sizeof(int64_t): valuesEqual = m_value.int64 == acRhs.m_value.int64;
    }

    return name == nameRhs && valuesEqual;
}

const RED4ext::CEnum* Enum::GetType() const
{
    return m_cpType;
}

const void* Enum::GetValuePtr() const
{
    return &m_value;
}
