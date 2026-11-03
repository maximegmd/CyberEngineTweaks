// CET macOS: TiltedCore's default allocator (MimallocAllocator) on the system malloc. The Windows build links TiltedCore with
// mimalloc; the macOS build does not carry mimalloc, so the same class is implemented here on malloc_zone (malloc / malloc_size /
// posix_memalign). Same interface, so TiltedCore's sources are unchanged.

#include <MimallocAllocator.hpp>

#include <cstdlib>
#include <malloc/malloc.h>

namespace TiltedPhoques
{
void* MimallocAllocator::Allocate(const size_t aSize) noexcept
{
    return std::malloc(aSize);
}

void MimallocAllocator::Free(void* apData) noexcept
{
    std::free(apData);
}

size_t MimallocAllocator::Size(void* apData) noexcept
{
    if (apData == nullptr)
        return 0;

    return malloc_size(apData);
}

void* MimallocAllocator::AlignedAllocate(size_t aSize, size_t aAlignment) noexcept
{
    void* data = nullptr;
    if (aAlignment < sizeof(void*))
        aAlignment = sizeof(void*);
    if (posix_memalign(&data, aAlignment, aSize) != 0)
        return nullptr;
    return data;
}

void MimallocAllocator::AlignedFree(void* apData) noexcept
{
    std::free(apData);
}
} // namespace TiltedPhoques
