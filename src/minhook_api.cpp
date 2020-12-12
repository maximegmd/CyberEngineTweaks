#include "common.hpp"
#include "minhook_api.hpp"
#include "minhook.h"

namespace minhook_api {

#if defined(USE_MINHOOK) && (USE_MINHOOK == 1)
void init() {
    if(MH_Initialize() != MH_OK) {
        DEBUG_TRACE("MH_Initialize : failed\n");
    }
}

void cleanup() {
    MH_Uninitialize();
}
#else
void init() {}
void cleanup() {}
#endif

} // namespace minhook


#if defined(USE_MINHOOK) && (USE_MINHOOK == 1)
#define D(funcname, ...)            \
    return funcname(__VA_ARGS__);   \
    __pragma(comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__))

extern "C" MH_STATUS WINAPI MH_Initialize_(void) {
                          D(MH_Initialize)
}

extern "C" MH_STATUS WINAPI MH_Uninitialize_(void) {
                          D(MH_Uninitialize);
}

extern "C" MH_STATUS WINAPI MH_CreateHook_(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal) {
                          D(MH_CreateHook, pTarget, pDetour, ppOriginal);
}

extern "C" MH_STATUS WINAPI MH_CreateHookApi_(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal) {
                          D(MH_CreateHookApi, pszModule, pszProcName, pDetour, ppOriginal);
}

extern "C" MH_STATUS WINAPI MH_CreateHookApiEx_(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID *ppOriginal, LPVOID *ppTarget) {
                          D(MH_CreateHookApiEx, pszModule, pszProcName, pDetour, ppOriginal, ppTarget);
}

extern "C" MH_STATUS WINAPI MH_RemoveHook_(LPVOID pTarget) {
                          D(MH_RemoveHook, pTarget);
}

extern "C" MH_STATUS WINAPI MH_EnableHook_(LPVOID pTarget) {
                          D(MH_EnableHook, pTarget);
}

extern "C" MH_STATUS WINAPI MH_DisableHook_(LPVOID pTarget) {
                          D(MH_DisableHook, pTarget);
}

extern "C" MH_STATUS WINAPI MH_QueueEnableHook_(LPVOID pTarget) {
                          D(MH_QueueEnableHook, pTarget);
}

extern "C" MH_STATUS WINAPI MH_QueueDisableHook_(LPVOID pTarget) {
                          D(MH_QueueDisableHook, pTarget);
}

extern "C" MH_STATUS WINAPI MH_ApplyQueued_(void) {
                          D(MH_ApplyQueued);
}

extern "C" const char * WINAPI MH_StatusToString_(MH_STATUS status) {
                             D(MH_StatusToString, status);
}
#endif
