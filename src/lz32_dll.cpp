#include "common.hpp"
#include "lz32_dll.hpp"
#include <array>
#include <cwchar>

///////////////////////////////////////////////////////////////
static HMODULE hModuleDll = nullptr;

namespace lz32_dll {

void loadGenuineDll(const wchar_t* systemDirectory) {
    unloadGenuineDll();

    const wchar_t dllName[] = L"lz32.dll";

    // systemDirectory : "C:\Windows\System32"
    // fullpathDllName : "C:\Windows\System32\lz32.dll"
    std::array<wchar_t, MAX_PATH> fullpathDllName;
    swprintf_s(fullpathDllName.data(), fullpathDllName.size(), L"%s\\%s", systemDirectory, dllName);

    // Load "genuine" lz32.dll
    hModuleDll = LoadLibraryW(fullpathDllName.data());
}

void unloadGenuineDll() {
    if(hModuleDll == nullptr) {
        return;
    }
    FreeLibrary(hModuleDll);
    hModuleDll = nullptr;
}

} // namespace lz32_dll


///////////////////////////////////////////////////////////////
template<typename T>
void setup(T*& funcPtr, const char* funcName) {
    if(funcPtr != nullptr) {
        return;
    }
    funcPtr = reinterpret_cast<T*>(GetProcAddress(hModuleDll, funcName));
}

#define D(funcname, ...)            \
    static decltype(funcname)* p;   \
    setup(p, #funcname);            \
    return p(__VA_ARGS__);          \
    __pragma(comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__))


extern "C" INT  WINAPI LZStart(VOID) {
    D(LZStart);
}

extern "C" VOID WINAPI LZDone(VOID) {
    D(LZDone);
}

extern "C" LONG WINAPI CopyLZFile(INT hfSource, INT hfDest) {
    D(CopyLZFile, hfSource, hfDest);
}

extern "C" LONG WINAPI LZCopy(INT hfSource, INT hfDest) {
    D(LZCopy, hfSource, hfDest);
}

extern "C" INT  WINAPI LZInit(INT hfSource) {
    D(LZInit, hfSource);
}

extern "C" INT  WINAPI GetExpandedNameA(LPSTR lpszSource, LPSTR lpszBuffer) {
    D(GetExpandedNameA, lpszSource, lpszBuffer);
}

extern "C" INT  WINAPI GetExpandedNameW(LPWSTR lpszSource, LPWSTR lpszBuffer) {
    D(GetExpandedNameW, lpszSource, lpszBuffer);
}

extern "C" INT  WINAPI LZOpenFileA(LPSTR lpFileName, LPOFSTRUCT lpReOpenBuf, WORD wStyle) {
    D(LZOpenFileA, lpFileName, lpReOpenBuf, wStyle);
}

extern "C" INT  WINAPI LZOpenFileW(LPWSTR lpFileName, LPOFSTRUCT lpReOpenBuf, WORD wStyle) {
    D(LZOpenFileW, lpFileName, lpReOpenBuf, wStyle);
}

extern "C" LONG WINAPI LZSeek(INT hFile, LONG lOffset, INT iOrigin) {
    D(LZSeek, hFile, lOffset, iOrigin);
}

extern "C" INT APIENTRY LZRead(INT hFile, CHAR* lpBuffer, INT cbRead) {
    D(LZRead, hFile, lpBuffer, cbRead);
}

extern "C" VOID APIENTRY LZClose(INT hFile) {
    D(LZClose,hFile);
}
