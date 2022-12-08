#pragma once

struct Fonts
{
    ~Fonts() = default;

    const std::tuple<const ImWchar*, std::filesystem::path> GetGlyphRange(std::string aLanguage);
    void BuildFonts(SIZE aOutSize);
    void RebuildFonts(ID3D12CommandQueue* apCommandQueue, SIZE aOutSize);
    void TriggerRebuildFonts();
    void LoadSystemFonts();

    const bool UseEmojiFont();

    ImFont* MainFont;
    ImFont* MonospaceFont;

private:
    friend struct CET;
    Fonts(Options& aOptions, Paths& aPaths);

    Options& m_options;
    Paths& m_paths;

    bool m_rebuildFonts{ false };
    bool m_useEmojiFont{ false };

    std::filesystem::path m_defaultMainFontPath{ L"NotoSans-Regular.ttf" };
    std::filesystem::path m_defaultMonospaceFontPath{ L"NotoSansMono-Regular.ttf" };
    std::filesystem::path m_defaultIconFontPath{ L"materialdesignicons.ttf" };
    std::filesystem::path m_defaultEmojiFontPath{ L"C:\\Windows\\Fonts\\seguiemj.ttf" }; // tried to use noto color emoji but it wont render. only this one works

    std::map<std::string, std::filesystem::path> m_defaultFonts{
      { "Default"                  , L"NotoSans-Regular.ttf"     },
      { "ChineseFull"              , L"NotoSansTC-Regular.otf"   },
      { "ChineseSimplifiedCommon"  , L"NotoSansSC-Regular.otf"   },
      { "Japanese"                 , L"NotoSansJP-Regular.otf"   },
      { "Korean"                   , L"NotoSansKR-Regular.otf"   },
      { "Cyrillic"                 , L"NotoSans-Regular.ttf"     },
      { "Thai"                     , L"NotoSansThai-Regular.ttf" },
      { "Vietnamese"               , L"NotoSans-Regular.ttf"     },
    };
};