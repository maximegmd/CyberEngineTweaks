#include <stdafx.h>

#include <string>

#include "Image.h"

#include "RED4ext/Api/v1/FileVer.hpp"
#include "RED4ext/Api/v1/SemVer.hpp"


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

#ifdef __APPLE__
#include "MacShims.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <RED4ext/Api/v1/Runtime.hpp>

// CET macOS: the macOS game has no PE version resource. The bundle's CFBundleShortVersionString is the product version (2.5.0 for
// game 2.5; 2.3.1 before); CET's address table is made for that build, so it is reported as the Windows file version of the latest
// build (RED4EXT_V1_RUNTIME_VERSION_LATEST) and anything else is left unknown (CET then refuses to start, as on Windows).
void Image::Initialize()
{
    base_address = CetMac::ImageBase();

    std::string shortVersion;
    if (CFBundleRef bundle = CFBundleGetMainBundle())
    {
        if (auto value = static_cast<CFStringRef>(CFBundleGetValueForInfoDictionaryKey(bundle, CFSTR("CFBundleShortVersionString"))))
        {
            char buffer[64] = {};
            if (CFStringGetCString(value, buffer, sizeof(buffer), kCFStringEncodingUTF8))
                shortVersion = buffer;
        }
    }

    unsigned major = 0, minor = 0, patch = 0;
    if (std::sscanf(shortVersion.c_str(), "%u.%u.%u", &major, &minor, &patch) >= 2)
        SemVersion = RED4EXT_V1_SEMVER(static_cast<uint8_t>(major), static_cast<uint16_t>(minor * 10 + patch), 0);

    if (shortVersion == "2.5.0")
        FileVersion = RED4EXT_V1_RUNTIME_VERSION_LATEST;

    CetMac::Log("[cet] game bundle version '%s' (%s)", shortVersion.c_str(),
                FileVersion.major != 0 ? "supported, same build as the address table" : "not supported by this build");
}
#else
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

            FileVersion = RED4EXT_V1_FILEVER(major, minor, build, revision);
        }

        {
            uint8_t major = (fileInfo->dwProductVersionMS >> 16) & 0xFF;
            uint16_t minor = fileInfo->dwProductVersionMS & 0xFFFF;
            uint32_t patch = (fileInfo->dwProductVersionLS >> 16) & 0xFFFF;

            SemVersion = RED4EXT_V1_SEMVER(major, minor, patch);
        }
    }
}
#endif
