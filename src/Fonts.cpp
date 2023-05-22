#include <stdafx.h>

#include <dwrite.h>
#include <imgui_freetype.h>

#include "Fonts.h"
#include "Utils.h"

#include <imgui_impl/dx12.h>

// Build Fonts
// if custom font not set:
//     we merge and use all the notosans fonts
//     (e.g. NotoSans-Regular.ttf + NotoSansJP-Regular.otf + ...).
//
// if custom font is set:
//     we use the custom font (e.g. c:/windows/fonts/Comic.ttf).
//
void Fonts::BuildFonts(const SIZE& acOutSize)
{
    // TODO - scale also by DPI
    const auto [resx, resy] = acOutSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    const auto& fontSettings = m_options.Font;
    const float fontSize = std::floorf(fontSettings.BaseSize * scaleFromReference);
    // scale emoji's fontsize by 0.8 to make the glyphs roughly the same size as the main font. not sure about other font, don't really have a good solution.
    const float emojiSize = fontSize * 0.8f;

    // get fontpaths
    // Get custom font paths from options
    const auto customMainFontPath = GetFontPathFromOption(fontSettings.FontMain);
    if (customMainFontPath.empty() && !fontSettings.FontMain.empty() && fontSettings.FontMain != "Default")
        Log::Error("Can't find custom main font at path: {}", GetAbsolutePath(fontSettings.FontMain, m_paths.Fonts(), true).string());

    const auto customMonospaceFontPath = GetFontPathFromOption(fontSettings.FontMonospace);
    if (customMonospaceFontPath.empty() && !fontSettings.FontMonospace.empty() && fontSettings.FontMonospace != "Default")
        Log::Error("Can't find custom main font at path: {}", GetAbsolutePath(fontSettings.FontMonospace, m_paths.Fonts(), true).string());

    const bool useCustomMainFont = !customMainFontPath.empty();
    const bool useCustomMonospaceFont = !customMonospaceFontPath.empty();

    // Set main font path to default if customMainFontPath is empty or doesnt exist.
    const auto mainFontPath = useCustomMainFont ? customMainFontPath : GetAbsolutePath(m_defaultMainFont, m_paths.Fonts(), false);
    if (mainFontPath.empty())
        Log::Error("Can't find default main font at path: {}, will use built-in font.", GetAbsolutePath(m_defaultMainFont, m_paths.Fonts(), true).string());

    // Set monospace font path to default if customMonospaceFontPath is empty or doesnt exist.
    const auto monospaceFontPath = useCustomMonospaceFont ? customMonospaceFontPath : GetAbsolutePath(m_defaultMonospaceFont, m_paths.Fonts(), false);
    if (monospaceFontPath.empty())
        Log::Error("Can't find default monospacee font at path: {}, will use built-in font.", GetAbsolutePath(m_defaultMonospaceFont, m_paths.Fonts(), true).string());

    const auto iconFontPath = GetAbsolutePath(m_defaultIconFont, m_paths.Fonts(), false);
    if (iconFontPath.empty())
        Log::Error("Can't find icon font at path: {}", GetAbsolutePath(m_defaultIconFont, m_paths.Fonts(), true).string());

    const auto emojiFontPath = GetAbsolutePath(m_defaultEmojiFont, m_paths.Fonts(), false);
    if (emojiFontPath.empty())
        Log::Error("Can't find emoji font at path: {}", GetAbsolutePath(m_defaultEmojiFont, m_paths.Fonts(), true).string());

    m_useEmojiFont = !emojiFontPath.empty();

    // create config for each font
    ImFontConfig fontConfig;
    fontConfig.OversampleH = fontSettings.OversampleHorizontal;
    fontConfig.OversampleV = fontSettings.OversampleVertical;
    static const ImWchar fontRange[] = {0x1, 0xFFFF, 0};

    ImFontConfig iconFontConfig;
    iconFontConfig.OversampleH = iconFontConfig.OversampleV = 1;
    iconFontConfig.GlyphMinAdvanceX = fontSize;
    static const ImWchar iconFontRange[] = {ICON_MIN_MD, ICON_MAX_MD, 0};

    ImFontConfig emojiFontConfig;
    emojiFontConfig.OversampleH = emojiFontConfig.OversampleV = 1;
    emojiFontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;
    static const ImWchar emojiFontRanges[] = {0x1, 0x1FFFF, 0};

    // load fonts without merging first, so we can calculate the offset to align the fonts.
    float mainFontBaselineDifference = 0.0f;
    float monospaceFontBaselineDifference = 0.0f;
    if (!mainFontPath.empty() && !monospaceFontPath.empty() && !iconFontPath.empty()) {
        auto mainFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &fontConfig);
        auto monospaceFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(monospaceFontPath.native()).c_str(), fontSize, &fontConfig);
        auto iconFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        io.Fonts->Build(); // Build atlas, retrieve pixel data.

        // calculate font baseline differences
        mainFontBaselineDifference = iconFont->Ascent - mainFont->Ascent;
        monospaceFontBaselineDifference = iconFont->Ascent - monospaceFont->Ascent;

        // clear fonts then merge
        io.Fonts->Clear();
    }

    // reconfig fonts for merge
    iconFontConfig.MergeMode = true;
    emojiFontConfig.MergeMode = true;

    // add main font
    {
        fontConfig.GlyphOffset.y = std::floorf(mainFontBaselineDifference * -0.5f);
        iconFontConfig.GlyphOffset.y = std::ceilf(mainFontBaselineDifference * 0.5f);

        if (!mainFontPath.empty())
            MainFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &fontConfig, fontRange);
        else
            MainFont = io.Fonts->AddFontDefault();

        // merge the default CJK fonts
        if (!useCustomMainFont)
        {
            fontConfig.MergeMode = true;
            for(const auto& font : m_defaultCJKFonts)
            {
                const std::filesystem::path fontPath = GetAbsolutePath(font, m_paths.Fonts(), false);
                if (fontPath.empty())
                {
                    Log::Error("Can't find font {}.", GetAbsolutePath(font, m_paths.Fonts(), true).string());
                    continue;
                }
                io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(fontPath.native()).c_str(), fontSize, &fontConfig, fontRange);
            }
            fontConfig.MergeMode = false;
        }

        // merge the icon font and emoji font
        if (!iconFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, emojiFontRanges);
    }

    // add monospace font
    {
        fontConfig.GlyphOffset.y = std::floorf(monospaceFontBaselineDifference * -0.5f);
        iconFontConfig.GlyphOffset.y = std::ceilf(monospaceFontBaselineDifference * 0.5f);

        if (!monospaceFontPath.empty())
            MonospaceFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(monospaceFontPath.native()).c_str(), fontSize, &fontConfig, fontRange);
        else
            MonospaceFont = io.Fonts->AddFontDefault();

        // merge main font with monospace font
        fontConfig.MergeMode = true;

        if (!mainFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &fontConfig, fontRange);

        // merge the default CJK fonts
        if (!useCustomMainFont)
        {
            for(const auto& font : m_defaultCJKFonts)
            {
                const std::filesystem::path fontPath = GetAbsolutePath(font, m_paths.Fonts(), false);
                if (fontPath.empty())
                    continue;
                io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(fontPath.native()).c_str(), fontSize, &fontConfig, fontRange);
            }
        }

        if (!iconFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, emojiFontRanges);
    }
}

// Rebuild font texture during runtime.
// Call before ImGui_ImplXXXX_NewFrame()
void Fonts::RebuildFonts(ID3D12CommandQueue* apCommandQueue, const SIZE& acOutSize)
{
    if (!m_rebuildFonts)
        return;

    BuildFonts(acOutSize);
    ImGui_ImplDX12_RecreateFontsTexture(apCommandQueue);

    m_rebuildFonts = false;
}

// Call from imgui to trgger RebuildFonts in the next frame
void Fonts::RebuildFontNextFrame()
{
    m_rebuildFonts = true;
}

// Check if the emoji font is loaded.
const bool Fonts::UseEmojiFont()
{
    return m_useEmojiFont;
}

Fonts::Fonts(Options& aOptions, Paths& aPaths)
    : m_options(aOptions)
    , m_paths(aPaths)
{
    EnumerateSystemFonts();
}

// Get system fonts and their path
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

    // print error message if failed
    if (FAILED(hresult))
        Log::Error("Error message: {}", std::system_category().message(hresult));
}

const std::vector<std::string>& Fonts::GetSystemFonts()
{
    return m_systemFonts;
}

std::filesystem::path Fonts::GetSystemFontPath(const std::string& acFontName)
{
    try
    {
        auto path = m_systemFontPaths.at(acFontName);
        if (exists(path))
            return path;
        else
            return {};
    }
    catch (...)
    {
        return {};
    }
}

// Returns absolute path, already checked for file existence.
std::filesystem::path Fonts::GetFontPathFromOption(const std::string& acFontOption)
{
    if (!GetSystemFontPath(acFontOption).empty())
        return GetSystemFontPath(acFontOption);

    if (!GetAbsolutePath(acFontOption, m_paths.Fonts(), false).empty())
        return GetAbsolutePath(acFontOption, m_paths.Fonts(), false);

    return {};
}