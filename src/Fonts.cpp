#include <stdafx.h>

#include "Fonts.h"

#include <Utils.h>
#include <imgui_impl/dx12.h>

GlyphRangesBuilder::GlyphRangesBuilder()
{
    const ImWchar defaultRange[] = {0x0020, 0x00FF, 0};
    m_builder.AddRanges(defaultRange);
    m_builder.AddChar(0xFFFD); // FallbackChar
    m_builder.AddChar(0x2026); // EllipsisChar
}

bool GlyphRangesBuilder::NeedsRebuild() const
{
    return m_needsRebuild;
}

void GlyphRangesBuilder::AddText(const std::string& acText)
{
    auto text = acText.c_str();
    while (*text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, NULL);
        text += c_len;
        if (c_len == 0)
            break;
        if (!m_builder.GetBit(c))
        {
            m_builder.AddChar((ImWchar)c);
            m_needsRebuild = true;
        }
    }
}

bool GlyphRangesBuilder::AddFile(const std::filesystem::path& acPath)
{
    std::ifstream file(acPath);
    if (!file)
        return false;

    std::stringstream buffer;
    buffer << file.rdbuf();

    AddText(buffer.str());

    file.close();

    return true;
}

void GlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* apRange)
{
    m_builder.BuildRanges(apRange);
    m_needsRebuild = false;
}

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
    const auto customMainFontPath = GetFontPathFromOption(fontSettings.MainFont);
    if (customMainFontPath.empty() && !fontSettings.MainFont.empty() && fontSettings.MainFont != "Default")
        Log::Error("Can't find custom main font at path: {}", GetAbsolutePath(fontSettings.MainFont, m_paths.Fonts(), true).string());

    const auto customMonosFontPath = GetFontPathFromOption(fontSettings.MonoFont);
    if (customMonosFontPath.empty() && !fontSettings.MonoFont.empty() && fontSettings.MonoFont != "Default")
        Log::Error("Can't find custom main font at path: {}", GetAbsolutePath(fontSettings.MonoFont, m_paths.Fonts(), true).string());

    const bool useCustomMainFont = !customMainFontPath.empty();
    const bool useCustomMonosFont = !customMonosFontPath.empty();

    const auto defaultMainFontPath = GetAbsolutePath(m_defaultMainFont, m_paths.Fonts(), false);
    const auto defaultMonoFontPath = GetAbsolutePath(m_defaultMonoFont, m_paths.Fonts(), false);

    // Set main font path to default if customMainFontPath is empty or doesnt exist.
    const auto mainFontPath = useCustomMainFont ? customMainFontPath : defaultMainFontPath;
    if (mainFontPath.empty())
        Log::Error("Can't find default main font at path: {}, will use built-in font.", GetAbsolutePath(m_defaultMainFont, m_paths.Fonts(), true).string());

    // Set monospace font path to default if customMonosFontPath is empty or doesnt exist.
    const auto monoFontPath = useCustomMonosFont ? customMonosFontPath : defaultMonoFontPath;
    if (monoFontPath.empty())
        Log::Error("Can't find default monospacee font at path: {}, will use built-in font.", GetAbsolutePath(m_defaultMonoFont, m_paths.Fonts(), true).string());

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

    static ImVector<ImWchar> fontRange;
    fontRange.clear();
    m_glyphRangesBuilder.BuildRanges(&fontRange);

    ImFontConfig iconFontConfig;
    iconFontConfig.OversampleH = iconFontConfig.OversampleV = 1;
    iconFontConfig.GlyphMinAdvanceX = fontSize;

    ImFontConfig emojiFontConfig;
    emojiFontConfig.OversampleH = emojiFontConfig.OversampleV = 1;
    emojiFontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

    // load fonts without merging first, so we can calculate the offset to align the fonts.
    float mainFontBaselineDifference = 0.0f;
    float monoFontBaselineDifference = 0.0f;
    if (!mainFontPath.empty() && !monoFontPath.empty() && !iconFontPath.empty())
    {
        static const ImWchar mainFontRange[] = {0x0041, 0x0041, 0};
        static const ImWchar iconFontRange[] = {0xF01C9, 0xF01C9, 0};
        auto mainFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &fontConfig, mainFontRange);
        auto monoFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(monoFontPath.native()).c_str(), fontSize, &fontConfig, mainFontRange);
        auto iconFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        io.Fonts->Build(); // Build atlas, retrieve pixel data.

        // calculate font baseline differences
        mainFontBaselineDifference = iconFont->Ascent - mainFont->Ascent;
        monoFontBaselineDifference = iconFont->Ascent - monoFont->Ascent;

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
            MainFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);
        else
            MainFont = io.Fonts->AddFontDefault();


        fontConfig.MergeMode = true;
        // merge NotoSans-Regular as fallback font when using custom font
        if (useCustomMainFont && !defaultMainFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(defaultMainFontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);

        // merge the default CJK fonts
        for (const auto& font : m_defaultCJKFonts)
        {
            const std::filesystem::path fontPath = GetAbsolutePath(font, m_paths.Fonts(), false);
            if (fontPath.empty())
            {
                Log::Error("Can't find font {}.", GetAbsolutePath(font, m_paths.Fonts(), true).string());
                continue;
            }
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(fontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);
        }
        fontConfig.MergeMode = false;

        // merge the icon font and emoji font
        if (!iconFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, fontRange.Data);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, fontRange.Data);
    }

    // add monospace font
    {
        fontConfig.GlyphOffset.y = std::floorf(monoFontBaselineDifference * -0.5f);
        iconFontConfig.GlyphOffset.y = std::ceilf(monoFontBaselineDifference * 0.5f);

        if (!monoFontPath.empty())
            MonoFont = io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(monoFontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);
        else
            MonoFont = io.Fonts->AddFontDefault();

        fontConfig.MergeMode = true;
        
        // merge NotoSans-Mono as fallback font when using custom monospace font
        if (useCustomMonosFont && !defaultMonoFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(defaultMonoFontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);

        // merge main font with monospace font
        if (!mainFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);

        // merge the default CJK fonts
        for (const auto& font : m_defaultCJKFonts)
        {
            const std::filesystem::path fontPath = GetAbsolutePath(font, m_paths.Fonts(), false);
            if (fontPath.empty())
                continue;
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(fontPath.native()).c_str(), fontSize, &fontConfig, fontRange.Data);
        }

        if (!iconFontPath.empty())
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, fontRange.Data);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, fontRange.Data);
    }
}

// Rebuild font texture during runtime.
// Call before ImGui_ImplXXXX_NewFrame()
void Fonts::RebuildFonts(ID3D12CommandQueue* apCommandQueue, const SIZE& acOutSize)
{

    if (m_rebuildFonts || m_glyphRangesBuilder.NeedsRebuild()) // Rebuild when font settings changed or glyph ranges changed
    {
        BuildFonts(acOutSize);
        ImGui_ImplDX12_RecreateFontsTexture(apCommandQueue);

        m_rebuildFonts = false;
    }
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

GlyphRangesBuilder& Fonts::GetGlyphRangesBuilder()
{
    return m_glyphRangesBuilder;
}

void Fonts::PrecacheGlyphsFromMods()
{
    const auto modsRoot = m_paths.ModsRoot();

    int fileCount = 0;

    for (const auto& modEntry : std::filesystem::directory_iterator(modsRoot))
    {
        // ignore normal files
        if (!modEntry.is_directory() && !modEntry.is_symlink())
            continue;
        // ignore mod using preserved name
        if (modEntry.path().native().starts_with(L"cet\\"))
            continue;

        const auto path = GetAbsolutePath(modEntry.path(), modsRoot, false);
        // ignore invalid path
        if (path.empty())
            continue;
        // ignore path symlinked to file
        if (!is_directory(path))
            continue;
        // ignore directory doesn't contain init.lua
        if (!exists(path / L"init.lua"))
            continue;

        // iterate all files within each mod folder recursively
        for (const auto& entry : std::filesystem::recursive_directory_iterator(modEntry, std::filesystem::directory_options::follow_directory_symlink))
        {
            const auto pathStr = UTF16ToUTF8(entry.path().native());

            // add the file path to the glyph range builder
            m_glyphRangesBuilder.AddText(pathStr);

            if (!entry.is_regular_file())
                continue;

            // ignore files without the following extension names: lua, txt, json, yml, yaml, toml, ini
            const std::vector<std::string> extensions = {".lua", ".txt", ".json", ".yml", ".yaml", ".toml", ".ini"};
            const std::string entryExtension = UTF16ToUTF8(entry.path().extension().native());
            if (std::find(extensions.begin(), extensions.end(), entryExtension) == extensions.end())
                continue;

            // open the file and add the content to the glyph range builder
            bool result = m_glyphRangesBuilder.AddFile(entry.path());

            if (!result)
            {
                Log::Error("Can't read file {}.", pathStr);
                continue;
            }

            fileCount++;
        }
    }

    Log::Info("Total mod files cached into glyph ranges builder: {}.", fileCount);
}

Fonts::Fonts(Options& aOptions, Paths& aPaths)
    : m_options(aOptions)
    , m_paths(aPaths)
{
    EnumerateSystemFonts();
    PrecacheGlyphsFromMods();
}