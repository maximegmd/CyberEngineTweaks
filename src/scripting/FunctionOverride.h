#pragma once

struct FunctionOverride
{
    FunctionOverride();
    ~FunctionOverride();

    static FunctionOverride& Get();

    void* MakeExecutable(uint8_t* apData, size_t aSize);

private:

    void* m_pBufferStart;
    void* m_pBuffer;
    size_t m_size{ 1 << 20 };
};