#include "Engine.h"

#include "Pattern.h"

CGameEngine* CGameEngine::Get()
{
    static auto* ptr = FindSignature({ 0x48, 0x89, 0x05, 0xCC, 0xCC, 0xCC, 0xCC,
                                                                    0x49, 0x8D, 0x9D, 0x88, 0x00, 0x00, 0x00,
                                                                    0x49, 0x8B, 0x07, 0x4C, 0x8B, 0xC3 }) + 3;

    return *reinterpret_cast<CGameEngine**>(ptr + *reinterpret_cast<int32_t*>(ptr) + 4);
}
