#include <stdafx.h>

#include <dwrite.h>
#include <imgui_freetype.h>

#include "Fonts.h"
#include "Utils.h"

#include <imgui_impl/dx12.h>

// Returns the glyph ranges for a given language and its default font.
// When aLanguage == "Full", returns full range of unicode plane 0.
// When aLanguage == "System", returns range according to windows language setting
const std::tuple<const ImWchar*, std::filesystem::path> Fonts::GetGlyphRange(std::string aLanguage)
{
    auto& io = ImGui::GetIO();

    if (aLanguage == "System")
    {
        switch (GetSystemDefaultLangID())
        {
        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL): aLanguage = "Traditional Chinese"; break;
        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED): aLanguage = "Simplified Chinese"; break;
        case MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT): aLanguage = "Japanese"; break;
        case MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT): aLanguage = "Korean"; break;
        case MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT):
        case MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT): aLanguage = "Cyrillic"; break;
        case MAKELANGID(LANG_THAI, SUBLANG_DEFAULT): aLanguage = "Thai"; break;
        case MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT): aLanguage = "Vietnamese"; break;
        }
    }

    if (aLanguage == "Traditional Chinese")
        return std::make_tuple(io.Fonts->GetGlyphRangesChineseFull(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    // Using GetGlyphRangesChineseFull() instead of GetGlyphRangesChineseSimplifiedCommon(), because the range is not comeplete.
    if (aLanguage == "Simplified Chinese")
        return std::make_tuple(io.Fonts->GetGlyphRangesChineseFull(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    if (aLanguage == "Japanese")
        return std::make_tuple(io.Fonts->GetGlyphRangesJapanese(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    if (aLanguage == "Korean")
        return std::make_tuple(io.Fonts->GetGlyphRangesKorean(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    if (aLanguage == "Cyrillic")
        return std::make_tuple(io.Fonts->GetGlyphRangesCyrillic(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    if (aLanguage == "Thai")
        return std::make_tuple(io.Fonts->GetGlyphRangesThai(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    if (aLanguage == "Vietnamese")
        return std::make_tuple(io.Fonts->GetGlyphRangesVietnamese(), GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));

    // add all glyphs from the font
    if (aLanguage == "Full")
    {
        static const ImWchar range[] = {0x1, 0xFFFF, 0};
        return std::make_tuple(range, GetAbsolutePath(GetDefaultLanguageFontPath(aLanguage), m_paths.Fonts(), false));
    }

    return std::make_tuple(io.Fonts->GetGlyphRangesDefault(), GetAbsolutePath(GetDefaultLanguageFontPath("Default"), m_paths.Fonts(), false));
}

// Build Fonts
// if custom font and language both not set:
//     we use default range with default fonts (e.g. RangeDefault(), NotoSans-Regular.ttf).
//
// if custom font not set but language is set:
//     we use the language range and its corresponding noto sans language fonts (e.g. RangeJapanese(), NotoSansJP-Regular.otf).
//
// if custom font is set but language not set:
//     we use the default range with the custom font (e.g. RnageDefault(), c:/windows/fonts/Comic.ttf).
//
// if custom font and language are both set:
//     we use the custom font with the language range (e.g. RangeChineseFull(), c:/windows/fonts/simhei.ttf).
//
void Fonts::BuildFonts(SIZE aOutSize)
{
    // TODO - scale also by DPI
    const auto [resx, resy] = aOutSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    const auto& fontSettings = m_options.Font;
    const float fontSize = std::floorf(fontSettings.BaseSize * scaleFromReference);
    // scale fontsize by 0.8 to make the glyphs roughly the same size as the main font. not sure about other font, don't really have a good solution.
    const float emojiSize = fontSize * 0.8f;

    // handle fontpaths and glyph ranges
    // Get custom font paths from options
    const auto customPath = GetFontPathFromOption(fontSettings.FontMain);
    const auto customMonospacePath = GetFontPathFromOption(fontSettings.FontMonospace);
    const bool useCustomMainFont = !customPath.empty();
    const bool useCustomMonospaceFont = !customMonospacePath.empty();

    // Set main font path to default if customPath is empty or doesnt exist.
    auto mainFontPath = useCustomMainFont ? customPath : GetAbsolutePath(m_defaultMainFontPath, m_paths.Fonts(), false);
    // Set monospace font path to default if customMonospacePath is empty or doesnt exist.
    const auto monospaceFontPath = useCustomMonospaceFont ? customMonospacePath : GetAbsolutePath(m_defaultMonospaceFontPath, m_paths.Fonts(), false);

    const auto iconFontPath = GetAbsolutePath(m_defaultIconFontPath, m_paths.Fonts(), false);
    const auto emojiFontPath = GetAbsolutePath(m_defaultEmojiFontPath, m_paths.Fonts(), false);
    m_useEmojiFont = !emojiFontPath.empty();

    const ImWchar* mainFontRange;

    if (useCustomMainFont)
        mainFontRange = std::get<0>(GetGlyphRange(fontSettings.Language));

    if (!useCustomMainFont)
    {
        std::tie(mainFontRange, mainFontPath) = GetGlyphRange(fontSettings.Language);
    }

    // create config for each font
    ImFontConfig mainFontConfig;
    mainFontConfig.OversampleH = fontSettings.OversampleHorizontal;
    mainFontConfig.OversampleV = fontSettings.OversampleVertical;

    ImFontConfig iconFontConfig;
    iconFontConfig.OversampleH = iconFontConfig.OversampleV = 1;
    iconFontConfig.GlyphMinAdvanceX = fontSize;
    static const ImWchar iconFontRange[] = {ICON_MIN_MD, ICON_MAX_MD, 0};

    ImFontConfig emojiFontConfig;
    emojiFontConfig.OversampleH = emojiFontConfig.OversampleV = 1;
    emojiFontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar emojiFontRanges[] = {0x1, 0x1FFFF, 0};

    // load fonts without merging first, so we can calculate the offset to align the fonts.
    auto mainFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &mainFontConfig);
    auto monospaceFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(monospaceFontPath.native()).c_str(), fontSize, &mainFontConfig);
    auto iconFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

    io.Fonts->Build(); // Build atlas, retrieve pixel data.

    // calculate font baseline differences
    const float mainFontBaselineDifference = iconFont->Ascent - mainFont->Ascent;
    const float monospaceFontBaselineDifference = iconFont->Ascent - monospaceFont->Ascent;

    // clear fonts then merge
    io.Fonts->Clear();

    // reconfig fonts for merge
    iconFontConfig.MergeMode = true;
    emojiFontConfig.MergeMode = true;

    // add main font
    {
        mainFontConfig.GlyphOffset.y = std::floorf(mainFontBaselineDifference * -0.5f);
        iconFontConfig.GlyphOffset.y = std::ceilf(mainFontBaselineDifference * 0.5f);

        MainFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &mainFontConfig, mainFontRange);

        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, emojiFontRanges);
    }

    // add monospace font
    {
        mainFontConfig.GlyphOffset.y = std::floorf(monospaceFontBaselineDifference * -0.5f);
        iconFontConfig.GlyphOffset.y = std::ceilf(monospaceFontBaselineDifference * 0.5f);

        MonospaceFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(monospaceFontPath.native()).c_str(), fontSize, &mainFontConfig, io.Fonts->GetGlyphRangesDefault());

        // merge custom main font with monospace font
        if (useCustomMainFont)
        {
            mainFontConfig.MergeMode = true;
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &mainFontConfig, mainFontRange);
        }

        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, emojiFontRanges);
    }
}

// Rebuild font texture during runtime.
// Call before ImGui_ImplXXXX_NewFrame()
void Fonts::RebuildFonts(ID3D12CommandQueue* apCommandQueue, SIZE aOutSize)
{
    if (!m_rebuildFonts)
        return;

    BuildFonts(aOutSize);
    ImGui_ImplDX12_RecreateFontsTexture(apCommandQueue);

    m_rebuildFonts = false;
}

// Call from imgui to trgger RebuildFonts in next frame
void Fonts::RebuildFontNextFrame()
{
    m_rebuildFonts = true;
}

const bool Fonts::UseEmojiFont()
{
    return m_useEmojiFont;
}

Fonts::Fonts(Options& aOptions, Paths& aPaths)
    : m_options(aOptions)
    , m_paths(aPaths)
{
    std::vector<std::pair<std::string, std::filesystem::path>> ranges = {
        {"Default", L"NotoSans-Regular.ttf"},
        {"System", ""},
        {"Full", L"NotoSans-Regular.ttf"},
        {"Cyrillic", L"NotoSans-Regular.ttf"},
        {"Japanese", L"NotoSansJP-Regular.otf"},
        {"Korean", L"NotoSansKR-Regular.otf"},
        {"Simplified Chinese", L"NotoSansSC-Regular.otf"},
        {"Traditional Chinese", L"NotoSansTC-Regular.otf"},
        {"Thai", L"NotoSansThai-Regular.ttf"},
        {"Vietnamese", L"NotoSans-Regular.ttf"},
    };
    for (const auto& range : ranges)
    {
        m_languages.emplace_back(range.first);
        m_defaultLanguageFontPaths.emplace(range.first, range.second);
    }

    EnumerateSystemFonts();
}

const std::vector<std::string>& Fonts::GetLanguages()
{
    return m_languages;
}

std::filesystem::path Fonts::GetDefaultLanguageFontPath(const std::string& acLanguages)
{
    try
    {
        return m_defaultLanguageFontPaths.at(acLanguages);
    }
    catch (...)
    {
        return {};
    }
}

void Fonts::EnumerateSystemFonts()
{
    IDWriteFactory* pDWriteFactory = NULL;
    HRESULT hresult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));

    IDWriteFontCollection* pFontCollection = NULL;

    // Get the system font collection.
    if (SUCCEEDED(hresult))
        hresult = pDWriteFactory->GetSystemFontCollection(&pFontCollection);

    UINT32 familyCount = 0;

    // Get the number of font families in the collection.
    if (SUCCEEDED(hresult))
        familyCount = pFontCollection->GetFontFamilyCount();

    for (UINT32 i = 0; i < familyCount; ++i)
    {
        IDWriteFontFamily* pFontFamily = NULL;

        // Get the font family.
        if (SUCCEEDED(hresult))
            hresult = pFontCollection->GetFontFamily(i, &pFontFamily);

        IDWriteLocalizedStrings* pFamilyNames = NULL;

        // Get a list of localized strings for the family name.
        if (SUCCEEDED(hresult))
            hresult = pFontFamily->GetFamilyNames(&pFamilyNames);

        UINT32 index = 0;
        BOOL exists = false;

        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];

        if (SUCCEEDED(hresult))
        {
            // Get the default locale for this user.
            int defaultLocaleSuccess = GetUserDefaultLocaleName(localeName, LOCALE_NAME_MAX_LENGTH);

            // If the default locale is returned, find that locale name, otherwise use "en-us".
            if (defaultLocaleSuccess)
                hresult = pFamilyNames->FindLocaleName(localeName, &index, &exists);

            if (SUCCEEDED(hresult) && !exists) // if the above find did not find a match, retry with US English
                hresult = pFamilyNames->FindLocaleName(L"en-us", &index, &exists);
        }

        // If the specified locale doesn't exist, select the first on the list.
        if (!exists)
            index = 0;

        UINT32 length = 0;

        // Get the string length.
        if (SUCCEEDED(hresult))
            hresult = pFamilyNames->GetStringLength(index, &length);

        // Allocate a string big enough to hold the name.
        wchar_t* name = new (std::nothrow) wchar_t[length + 1];
        if (name == NULL)
            hresult = E_OUTOFMEMORY;

        // Get the family name.
        if (SUCCEEDED(hresult))
            hresult = pFamilyNames->GetString(index, name, length + 1);

        // Get font
        IDWriteFont* pFont = NULL;
        if (SUCCEEDED(hresult))
            hresult = pFontFamily->GetFont(index, &pFont);

        // Get fontface
        IDWriteFontFace* pFontFace = NULL;
        if (SUCCEEDED(hresult))
            hresult = pFont->CreateFontFace(&pFontFace);

        UINT32 numberOfFiles = 0;
        if (SUCCEEDED(hresult))
            hresult = pFontFace->GetFiles(&numberOfFiles, NULL);

        IDWriteFontFile* pFontFiles = NULL;
        if (SUCCEEDED(hresult))
            hresult = pFontFace->GetFiles(&numberOfFiles, &pFontFiles);

        if (numberOfFiles > 0)
        {
            IDWriteFontFileLoader* pFileLoader = NULL;
            IDWriteLocalFontFileLoader* pLocalFileLoader = NULL;
            const void* pFontFileReferenceKey = NULL;
            UINT32 fontFileReferenceKeySize = 0;

            if (SUCCEEDED(hresult))
                hresult = pFontFiles[0].GetLoader(&pFileLoader);

            if (SUCCEEDED(hresult))
                hresult = pFontFiles[0].GetReferenceKey(&pFontFileReferenceKey, &fontFileReferenceKeySize);

            if (SUCCEEDED(hresult))
                hresult = pFileLoader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), (void**)&pLocalFileLoader);

            UINT32 filePathLength = 0;
            if (SUCCEEDED(hresult))
                hresult = pLocalFileLoader->GetFilePathLengthFromKey(pFontFileReferenceKey, fontFileReferenceKeySize, &filePathLength);

            WCHAR* fontPath = new (std::nothrow) WCHAR[filePathLength + 1];
            if (SUCCEEDED(hresult))
                hresult = pLocalFileLoader->GetFilePathFromKey(pFontFileReferenceKey, fontFileReferenceKeySize, fontPath, filePathLength + 1);

            if (SUCCEEDED(hresult))
            {
                m_systemFonts.emplace_back(UTF16ToUTF8(name));
                m_systemFontPaths.emplace(UTF16ToUTF8(name), UTF16ToUTF8(fontPath));
            }
        }
    }
}

const std::vector<std::string>& Fonts::GetSystemFonts()
{
    return m_systemFonts;
}

std::filesystem::path Fonts::GetSystemFontPath(const std::string& acFontName)
{
    try
    {
        return m_systemFontPaths.at(acFontName);
    }
    catch (...)
    {
        return {};
    }
}

std::filesystem::path Fonts::GetFontPathFromOption(const std::string& acFontOption)
{
    if (!GetSystemFontPath(acFontOption).empty())
        return GetSystemFontPath(acFontOption);

    if (!GetAbsolutePath(acFontOption, m_paths.Fonts(), false).empty())
        return GetAbsolutePath(acFontOption, m_paths.Fonts(), false);

    return {};
}