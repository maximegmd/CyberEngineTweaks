#include <stdafx.h>

#include <string>

#include "Image.h"

#include "RED4ext/Api/FileVer.hpp"


struct PdbInfo
{
    DWORD Signature;
    BYTE Guid[16];
    DWORD Age;
    char PdbFileName[1];
};

struct ImageVersion
{
    uint8_t Guid[16];
    uint64_t Version;
};

void Image::Initialize()
{
    std::wstring fileName;
    TCHAR exePathBuf[MAX_PATH] = {0};
    GetModuleFileNameW(GetModuleHandle(nullptr), exePathBuf, static_cast<DWORD>(std::size(exePathBuf)));

    fileName = exePathBuf;

    auto size = GetFileVersionInfoSize(fileName.c_str(), nullptr);
    if (!size)
    {
        return;
    }

    std::unique_ptr<uint8_t[]> data(new (std::nothrow) uint8_t[size]());
    if (!data)
    {
        return;
    }

    if (!GetFileVersionInfo(fileName.c_str(), 0, size, data.get()))
    {
        return;
    }

    struct LangAndCodePage
    {
        WORD language;
        WORD codePage;
    }* translations;
    uint32_t translationsBytes;

    if (!VerQueryValue(data.get(), L"\\VarFileInfo\\Translation", reinterpret_cast<void**>(&translations), &translationsBytes))
    {
        return;
    }

    bool isGame = false;

    for (uint32_t i = 0; i < (translationsBytes / sizeof(LangAndCodePage)); i++)
    {
        wchar_t* productName;
        auto subBlock = fmt::format(L"\\StringFileInfo\\{:04x}{:04x}\\ProductName", translations[i].language, translations[i].codePage);

        if (VerQueryValue(data.get(), subBlock.c_str(), reinterpret_cast<void**>(&productName), &translationsBytes))
        {
            constexpr std::wstring_view expectedProductName = L"Cyberpunk 2077";
            if (productName == expectedProductName)
            {
                isGame = true;
                break;
            }
        }
    }

    if (isGame)
    {
        VS_FIXEDFILEINFO* fileInfo = nullptr;
        UINT fileInfoBytes;

        if (!VerQueryValue(data.get(), L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoBytes))
        {
            return;
        }

        constexpr auto signature = 0xFEEF04BD;
        if (fileInfo->dwSignature != signature)
        {
            return;
        }

        {
            uint16_t major = (fileInfo->dwFileVersionMS >> 16) & 0xFF;
            uint16_t minor = fileInfo->dwFileVersionMS & 0xFFFF;
            uint16_t build = (fileInfo->dwFileVersionLS >> 16) & 0xFFFF;
            uint16_t revision = fileInfo->dwFileVersionLS & 0xFFFF;

            FileVersion = RED4EXT_FILEVER(major, minor, build, revision);
        }

        {
            uint8_t major = (fileInfo->dwProductVersionMS >> 16) & 0xFF;
            uint16_t minor = fileInfo->dwProductVersionMS & 0xFFFF;
            uint32_t patch = (fileInfo->dwProductVersionLS >> 16) & 0xFFFF;

            SemVersion = RED4EXT_SEMVER(major, minor, patch);
        }
    }
}
