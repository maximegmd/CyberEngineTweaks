#include <stdafx.h>

#include "FunctionOverride.h"

FunctionOverride::FunctionOverride()
{
    m_pBuffer = m_pBufferStart = VirtualAlloc(nullptr, m_size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

FunctionOverride::~FunctionOverride()
{
    VirtualFree(m_pBufferStart, 0, MEM_RELEASE);
}

FunctionOverride& FunctionOverride::Get()
{
    static FunctionOverride s_instance;
    return s_instance;
}

void* FunctionOverride::MakeExecutable(uint8_t* apData, size_t aSize)
{
    if (std::align(0x10, aSize, m_pBuffer, m_size))
    {
        uint8_t* result = static_cast<uint8_t*>(m_pBuffer);
        m_pBuffer = static_cast<char*>(m_pBuffer) + aSize;
        m_size -= aSize;

        std::memcpy(result, apData, aSize);

        return result;
    }

    return nullptr;
}
