#pragma once

struct FunctionOverride
{
    FunctionOverride();
    ~FunctionOverride();

    static FunctionOverride& Get();

    void* MakeExecutable(uint8_t* apData, size_t aSize);

private:

	struct Overrides
    {
        std::string Name;
        TiltedPhoques::Map<uint64_t, int> Functions;
    };

    TiltedPhoques::Map<uint64_t, Overrides> m_typeToOverrides;
    void* m_pBufferStart;
    void* m_pBuffer;
    size_t m_size{ 1 << 20 };
};