#include <stdafx.h>

#include <imgui_freetype.h>

#include "Fonts.h"
#include "Utils.h"

#include <imgui_impl/dx12.h>

// Returns the glyph ranges for a given language and its default font.
// When acLanguage == "Full", returns full range of unicode plane 0.
// When acLanguage == "System", returns range according to windows language setting
const std::tuple<const ImWchar*, std::filesystem::path> Fonts::GetGlyphRange(std::string aLanguage)
{
    auto& io = ImGui::GetIO();

    if (aLanguage == "System")
    {
        switch (GetSystemDefaultLangID())
        {
        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL):
            aLanguage = "Traditional Chinese";
            break;
        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED):
            aLanguage = "Simplified Chinese";
            break;
        case MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT):
            aLanguage = "Japanese";
            break;
        case MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT):
            aLanguage = "Korean";
            break;
        case MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT):
        case MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT):
            aLanguage = "Cyrillic";
            break;
        case MAKELANGID(LANG_THAI, SUBLANG_DEFAULT):
            aLanguage = "Thai";
            break;
        case MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT):
            aLanguage = "Vietnamese";
            break;
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
        static const ImWchar range[] = { 0x1, 0xFFFF, 0 };
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
    const auto customPath = fontSettings.Path.empty() ? std::filesystem::path{} : GetAbsolutePath(fontSettings.Path, m_paths.Fonts(), false);
    const auto customMonospacePath = fontSettings.MonospacePath.empty() ? std::filesystem::path{} : GetAbsolutePath(fontSettings.MonospacePath, m_paths.Fonts(), false);
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
    static const ImWchar emojiFontRanges[] = { 0x1, 0x1FFFF, 0 };

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
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(mainFontPath.native()).c_str(), fontSize, &mainFontConfig, mainFontRange);
        
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(iconFontPath.native()).c_str(), fontSize, &iconFontConfig, iconFontRange);

        if (m_useEmojiFont)
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(emojiFontPath.native()).c_str(), emojiSize, &emojiFontConfig, emojiFontRanges);
    }
}

// Rebuild font texture during runtime.
// Call before ImGui_ImplXXXX_NewFrame()
void Fonts::RebuildFonts(ID3D12CommandQueue* apCommandQueue, SIZE aOutSize)
{
    if (!m_rebuildFonts) return;

    BuildFonts(aOutSize);
    ImGui_ImplDX12_RecreateFontsTexture(apCommandQueue);

    m_rebuildFonts = false;
}

// Call from imgui to trgger RebuildFonts in next frame
void Fonts::TriggerFontRebuild()
{
    m_rebuildFonts = true;
}

// TODO load system font use dwrite
void Fonts::LoadSystemFonts()
{

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
    { "Default"                  , L"NotoSans-Regular.ttf"     },
    { "System"                   , ""                          },
    { "Full"                     , L"NotoSans-Regular.ttf"     },
    { "Cyrillic"                 , L"NotoSans-Regular.ttf"     },
    { "Japanese"                 , L"NotoSansJP-Regular.otf"   },
    { "Korean"                   , L"NotoSansKR-Regular.otf"   },
    { "Simplified Chinese"       , L"NotoSansSC-Regular.otf"   },
    { "Traditional Chinese"      , L"NotoSansTC-Regular.otf"   },
    { "Thai"                     , L"NotoSansThai-Regular.ttf" },
    { "Vietnamese"               , L"NotoSans-Regular.ttf"     },
  };
  for(const auto& range : ranges)
  {
    m_languages.emplace_back(range.first);
    m_defaultLanguageFontPaths.emplace(range.first, range.second);
  }
}

const std::vector<std::string>& Fonts::GetLanguages()
{
    return m_languages;
}

const std::filesystem::path& Fonts::GetDefaultLanguageFontPath(const std::string& acLanguages)
{
    try
    {
        return m_defaultLanguageFontPaths.at(acLanguages);
    }
    catch(const std::exception& e)
    {
        return { };
    }
}