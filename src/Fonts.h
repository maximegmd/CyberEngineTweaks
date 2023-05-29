#pragma once

// a wrapper for ImFontGlyphRangesBuilder
struct GlyphRangesBuilder
{
    GlyphRangesBuilder();
    bool NeedsRebuild() const;
    void AddText(const std::string& acText);
    bool AddFile(const std::filesystem::path& acPath);
    void BuildRanges(ImVector<ImWchar>* apRange);

private:
    bool m_needsRebuild{false};
    ImFontGlyphRangesBuilder m_builder;
};

struct Font
{
    Font(const std::filesystem::path& acPath);
    Font(const std::string& acName, const std::filesystem::path& acPath);
    std::string GetName() const;
    std::filesystem::path GetPath() const;
    bool Exists();

private:
    std::string m_name;
    std::filesystem::path m_path;
};

struct Fonts
{
    ~Fonts() = default;

    void BuildFonts(const SIZE& acOutSize);
    void RebuildFonts(ID3D12CommandQueue* apCommandQueue, const SIZE& acOutSize);
    void RebuildFontNextFrame();

    const bool UseEmojiFont();

    void EnumerateSystemFonts();
    const std::vector<Font>& GetSystemFonts();
    Font GetSystemFont(const std::string& acFontName) const;
    std::filesystem::path GetFontPathFromOption(const std::string& acFontOption) const;

    GlyphRangesBuilder& GetGlyphRangesBuilder();
    void PrecacheModFiles();
    void PrecacheLanguageFiles();

    ImFont* MainFont;
    ImFont* MonoFont;

private:
    friend struct CET;
    Fonts(Options& aOptions, Paths& aPaths);

    Options& m_options;
    Paths& m_paths;

    GlyphRangesBuilder m_glyphRangesBuilder;

    bool m_rebuildFonts{false};
    bool m_useEmojiFont{false};
    std::vector<Font> m_systemFonts;

    std::filesystem::path m_defaultMainFont{L"NotoSans-Regular.ttf"};
    std::vector<std::filesystem::path> m_defaultCJKFonts{
        L"NotoSansJP-Regular.otf", L"NotoSansKR-Regular.otf", L"NotoSansSC-Regular.otf", L"NotoSansTC-Regular.otf", L"NotoSansThai-Regular.ttf"};
    std::filesystem::path m_defaultMonoFont{L"NotoSansMono-Regular.ttf"};
    std::filesystem::path m_defaultIconFont{L"materialdesignicons.ttf"};
    std::filesystem::path m_defaultEmojiFont{L"C:\\Windows\\Fonts\\seguiemj.ttf"}; // tried to use noto color emoji but it wont render. only this one works
};
