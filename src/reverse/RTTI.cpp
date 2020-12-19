#include "RTTI.h"

#include "Pattern.h"

using TCClass_GetFunction = CClassFunction * (CRTTIBaseType* apBase, uint64_t aHash);
using TCRTTISystem_Get = CRTTISystem*();

CClassFunction* CClass::GetFunction(uint64_t aName)
{
    static auto* RealCClass_GetFunction = reinterpret_cast<TCClass_GetFunction*>(FindSignature(
        {
            0x4C, 0x8B, 0xC9, 0x48, 0x85, 0xC9, 0x74, 0x5E,
            0x49, 0x8B, 0x41, 0x48
        }));

    if (RealCClass_GetFunction)
        return RealCClass_GetFunction(this, aName);

    return nullptr;
}

CRTTISystem* CRTTISystem::Get()
{
    static auto* RealCRTTISystem_Get = (TCRTTISystem_Get*)FindSignature({ 0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x65, 0x48,
                                                          0x8B, 0x04, 0x25, 0x58, 0x00, 0x00, 0x00, 0x48,
                                                          0x8D, 0x1D, 0xCC, 0xCC, 0xCC, 0xCC });

    return RealCRTTISystem_Get();
}
