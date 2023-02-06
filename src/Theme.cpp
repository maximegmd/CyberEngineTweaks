#include "stdafx.h"

#include <fmt/core.h>
#include <scn/scn.h>

#include "Utils.h"

#include "Theme.h"

using nlohmann::json;

// Helpers

static json XyDimensionsToObject(const ImVec2& aXy)
{
    return json::object({{"x", aXy.x}, {"y", aXy.y}});
}

static ImVec2 XyDimensionsFromObject(const json& aJson, const ImVec2& aDefaults)
{
    return ImVec2(aJson.value("x", aDefaults.x), aJson.value("y", aDefaults.y));
}

static json DirectionToLabel(const ImGuiDir& aDirection)
{
    switch (aDirection)
    {
    case ImGuiDir_None: return "None";
    case ImGuiDir_Left: return "Left";
    case ImGuiDir_Right: return "Right";
    case ImGuiDir_Up: return "Up";
    case ImGuiDir_Down: return "Down";
    default: return "Unknown";
    }
}

static ImGuiDir DirectionFromLabel(const std::string& aDirectionLabel, const ImGuiDir& aDefaultDirection)
{
    if (aDirectionLabel == "None")
        return ImGuiDir_None;
    if (aDirectionLabel == "Left")
        return ImGuiDir_Left;
    if (aDirectionLabel == "Right")
        return ImGuiDir_Right;
    if (aDirectionLabel == "Up")
        return ImGuiDir_Up;
    if (aDirectionLabel == "Down")
        return ImGuiDir_Down;

    return aDefaultDirection;
}

static std::string ColorToHex(const ImVec4& aColor)
{
    return fmt::format(
        //    R     G     B     A
        "#{:02x}{:02x}{:02x}{:02x}", static_cast<int>(aColor.x * 255), static_cast<int>(aColor.y * 255), static_cast<int>(aColor.z * 255), static_cast<int>(aColor.w * 255));
}

static ImVec4 ColorFromHex(const std::string& aHex, const ImVec4& aDefaultColor)
{
    if (aHex.empty())
    {
        return aDefaultColor;
    }

    int r, g, b = 0;

    auto foundRGB = scn::scan(aHex, "#{:2x}{:2x}{:2x}", r, g, b);

    if (!foundRGB)
    {
        Log::Warn("Failed to parse color, must be hex with optional alpha (\"#00ff00\", \"#00ff00ff\"): {}", aHex);
        return aDefaultColor;
    }

    int a = 255;
    auto alphaResult = scn::scan(foundRGB.range(), "{:2x}", a);

    ImVec4 outColor{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};

    return outColor;
}

//
// Codec
//

// Encoders
using EncodeValueFunc = std::function<void(const ImGuiStyle&, json& aOutJson)>;

// Decoders
using DecodeValueFunc = std::function<void(ImGuiStyle& aOutStyle, const json& aJson)>;

struct Codec
{
    std::string valueName;
    EncodeValueFunc encodeFrom;
    DecodeValueFunc decodeInto;
};

//
// Simple scalar types
//

template <typename TAccessor> static EncodeValueFunc EncodeScalar(std::string aValueName, TAccessor aValueAccessor)
{
    return [=](const ImGuiStyle& aStyle, json& aOutJson) -> void
    {
        aOutJson[aValueName] = aStyle.*aValueAccessor;
    };
}

template <typename TAccessor> static DecodeValueFunc DecodeScalar(std::string aValueName, TAccessor aValueAccessor)
{
    return [=](ImGuiStyle& aOutStyle, const json& aJson) -> void
    {
        aOutStyle.*aValueAccessor = aJson.value(aValueName, aOutStyle.*aValueAccessor);
    };
}

template <typename TAccessor> static Codec Scalar(std::string aValueName, TAccessor aValueAccessor)
{
    return Codec{aValueName, EncodeScalar(aValueName, aValueAccessor), DecodeScalar(aValueName, aValueAccessor)};
}

//
// ImVec2 dimension types with X and Y values
//

template <typename TAccessor> static EncodeValueFunc EncodeXyDimensions(std::string aValueName, TAccessor aValueAccessor)
{
    return [=](const ImGuiStyle& aStyle, json& aOutJson) -> void
    {
        aOutJson[aValueName] = XyDimensionsToObject(aStyle.*aValueAccessor);
    };
}

template <typename TAccessor> static DecodeValueFunc DecodeXyDimensions(std::string aValueName, TAccessor aValueAccessor)
{
    return [=](ImGuiStyle& aOutStyle, const json& aJson) -> void
    {
        aOutStyle.*aValueAccessor = XyDimensionsFromObject(aJson.value(aValueName, json::object()), aOutStyle.*aValueAccessor);
    };
}

template <typename TAccessor> static Codec XyDimensions(std::string aValueName, TAccessor aValueAccessor)
{
    return Codec{aValueName, EncodeXyDimensions(aValueName, aValueAccessor), DecodeXyDimensions(aValueName, aValueAccessor)};
}

//
// ImGuiDir[ection] types
//

template <typename TAccessor> static EncodeValueFunc EncodeDirection(std::string aValueName, TAccessor aValueAccessor)
{
    return [=](const ImGuiStyle& aStyle, json& aOutJson) -> void
    {
        aOutJson[aValueName] = DirectionToLabel(aStyle.*aValueAccessor);
    };
}

template <typename TAccessor> static DecodeValueFunc DecodeDirection(std::string aValueName, TAccessor aValueAccessor)
{
    return [=](ImGuiStyle& aOutStyle, const json& aJson) -> void
    {
        aOutStyle.*aValueAccessor = DirectionFromLabel(aJson.value(aValueName, ""), aOutStyle.*aValueAccessor);
    };
}

template <typename TAccessor> static Codec Direction(std::string aValueName, TAccessor aValueAccessor)
{
    return Codec{aValueName, EncodeDirection(aValueName, aValueAccessor), DecodeDirection(aValueName, aValueAccessor)};
}

//
// Colors
//

static EncodeValueFunc EncodeColor(std::string aValueName, const ImGuiCol_ aColorableIndex)
{
    return [=](const ImGuiStyle& aStyle, json& aOutJson) -> void
    {
        aOutJson[aValueName] = ColorToHex(aStyle.Colors[aColorableIndex]);
    };
}

static DecodeValueFunc DecodeColor(std::string aValueName, const ImGuiCol_ aColorableIndex)
{
    return [=](ImGuiStyle& aOutStyle, const json& aJson) -> void
    {
        aOutStyle.Colors[aColorableIndex] = ColorFromHex(aJson.value(aValueName, ""), aOutStyle.Colors[aColorableIndex]);
    };
}

static Codec Color(std::string aValueName, ImGuiCol_ aColorableIndex)
{
    return Codec{aValueName, EncodeColor(aValueName, aColorableIndex), DecodeColor(aValueName, aColorableIndex)};
}

//
// Full codecs
//

static const auto StyleCodecs = std::forward_list{
    Scalar("Alpha", &ImGuiStyle::Alpha),
    Scalar("DisabledAlpha", &ImGuiStyle::DisabledAlpha),
    XyDimensions("WindowPadding", &ImGuiStyle::WindowPadding),
    Scalar("WindowRounding", &ImGuiStyle::WindowRounding),
    Scalar("WindowBorderSize", &ImGuiStyle::WindowBorderSize),
    XyDimensions("WindowMinSize", &ImGuiStyle::WindowMinSize),
    XyDimensions("WindowTitleAlign", &ImGuiStyle::WindowTitleAlign),
    Direction("WindowMenuButtonPosition", &ImGuiStyle::WindowMenuButtonPosition),
    Scalar("ChildRounding", &ImGuiStyle::ChildRounding),
    Scalar("ChildBorderSize", &ImGuiStyle::ChildBorderSize),
    Scalar("PopupRounding", &ImGuiStyle::PopupRounding),
    Scalar("PopupBorderSize", &ImGuiStyle::PopupBorderSize),
    XyDimensions("FramePadding", &ImGuiStyle::FramePadding),
    Scalar("FrameRounding", &ImGuiStyle::FrameRounding),
    Scalar("FrameBorderSize", &ImGuiStyle::FrameBorderSize),
    XyDimensions("ItemSpacing", &ImGuiStyle::ItemSpacing),
    XyDimensions("ItemInnerSpacing", &ImGuiStyle::ItemInnerSpacing),
    XyDimensions("CellPadding", &ImGuiStyle::CellPadding),
    XyDimensions("TouchExtraPadding", &ImGuiStyle::TouchExtraPadding),
    Scalar("IndentSpacing", &ImGuiStyle::IndentSpacing),
    Scalar("ColumnsMinSpacing", &ImGuiStyle::ColumnsMinSpacing),
    Scalar("ScrollbarSize", &ImGuiStyle::ScrollbarSize),
    Scalar("ScrollbarRounding", &ImGuiStyle::ScrollbarRounding),
    Scalar("GrabMinSize", &ImGuiStyle::GrabMinSize),
    Scalar("GrabRounding", &ImGuiStyle::GrabRounding),
    Scalar("LogSliderDeadzone", &ImGuiStyle::LogSliderDeadzone),
    Scalar("TabRounding", &ImGuiStyle::TabRounding),
    Scalar("TabBorderSize", &ImGuiStyle::TabBorderSize),
    Scalar("TabMinWidthForCloseButton", &ImGuiStyle::TabMinWidthForCloseButton),
    Direction("ColorButtonPosition", &ImGuiStyle::ColorButtonPosition),
    XyDimensions("ButtonTextAlign", &ImGuiStyle::ButtonTextAlign),
    XyDimensions("SelectableTextAlign", &ImGuiStyle::SelectableTextAlign),
    XyDimensions("DisplayWindowPadding", &ImGuiStyle::DisplayWindowPadding),
    XyDimensions("DisplaySafeAreaPadding", &ImGuiStyle::DisplaySafeAreaPadding),
    Scalar("MouseCursorScale", &ImGuiStyle::MouseCursorScale),
    Scalar("AntiAliasedLines", &ImGuiStyle::AntiAliasedLines),
    Scalar("AntiAliasedLinesUseTex", &ImGuiStyle::AntiAliasedLinesUseTex),
    Scalar("AntiAliasedFill", &ImGuiStyle::AntiAliasedFill),
    Scalar("CurveTessellationTol", &ImGuiStyle::CurveTessellationTol),
    Scalar("CircleTessellationMaxError", &ImGuiStyle::CircleTessellationMaxError),
};

static const auto ColorCodecs = std::forward_list{
    Color("Text", ImGuiCol_Text),
    Color("TextDisabled", ImGuiCol_TextDisabled),
    Color("WindowBg", ImGuiCol_WindowBg),
    Color("ChildBg", ImGuiCol_ChildBg),
    Color("PopupBg", ImGuiCol_PopupBg),
    Color("Border", ImGuiCol_Border),
    Color("BorderShadow", ImGuiCol_BorderShadow),
    Color("FrameBg", ImGuiCol_FrameBg),
    Color("FrameBgHovered", ImGuiCol_FrameBgHovered),
    Color("FrameBgActive", ImGuiCol_FrameBgActive),
    Color("TitleBg", ImGuiCol_TitleBg),
    Color("TitleBgActive", ImGuiCol_TitleBgActive),
    Color("TitleBgCollapsed", ImGuiCol_TitleBgCollapsed),
    Color("MenuBarBg", ImGuiCol_MenuBarBg),
    Color("ScrollbarBg", ImGuiCol_ScrollbarBg),
    Color("ScrollbarGrab", ImGuiCol_ScrollbarGrab),
    Color("ScrollbarGrabHovered", ImGuiCol_ScrollbarGrabHovered),
    Color("ScrollbarGrabActive", ImGuiCol_ScrollbarGrabActive),
    Color("CheckMark", ImGuiCol_CheckMark),
    Color("SliderGrab", ImGuiCol_SliderGrab),
    Color("SliderGrabActive", ImGuiCol_SliderGrabActive),
    Color("Button", ImGuiCol_Button),
    Color("ButtonHovered", ImGuiCol_ButtonHovered),
    Color("ButtonActive", ImGuiCol_ButtonActive),
    Color("Header", ImGuiCol_Header),
    Color("HeaderHovered", ImGuiCol_HeaderHovered),
    Color("HeaderActive", ImGuiCol_HeaderActive),
    Color("Separator", ImGuiCol_Separator),
    Color("SeparatorHovered", ImGuiCol_SeparatorHovered),
    Color("SeparatorActive", ImGuiCol_SeparatorActive),
    Color("ResizeGrip", ImGuiCol_ResizeGrip),
    Color("ResizeGripHovered", ImGuiCol_ResizeGripHovered),
    Color("ResizeGripActive", ImGuiCol_ResizeGripActive),
    Color("Tab", ImGuiCol_Tab),
    Color("TabHovered", ImGuiCol_TabHovered),
    Color("TabActive", ImGuiCol_TabActive),
    Color("TabUnfocused", ImGuiCol_TabUnfocused),
    Color("TabUnfocusedActive", ImGuiCol_TabUnfocusedActive),
    Color("DockingPreview", ImGuiCol_DockingPreview),
    Color("DockingEmptyBg", ImGuiCol_DockingEmptyBg),
    Color("PlotLines", ImGuiCol_PlotLines),
    Color("PlotLinesHovered", ImGuiCol_PlotLinesHovered),
    Color("PlotHistogram", ImGuiCol_PlotHistogram),
    Color("PlotHistogramHovered", ImGuiCol_PlotHistogramHovered),
    Color("TableHeaderBg", ImGuiCol_TableHeaderBg),
    Color("TableBorderStrong", ImGuiCol_TableBorderStrong),
    Color("TableBorderLight", ImGuiCol_TableBorderLight),
    Color("TableRowBg", ImGuiCol_TableRowBg),
    Color("TableRowBgAlt", ImGuiCol_TableRowBgAlt),
    Color("TextSelectedBg", ImGuiCol_TextSelectedBg),
    Color("DragDropTarget", ImGuiCol_DragDropTarget),
    Color("NavHighlight", ImGuiCol_NavHighlight),
    Color("NavWindowingHighlight", ImGuiCol_NavWindowingHighlight),
    Color("NavWindowingDimBg", ImGuiCol_NavWindowingDimBg),
    Color("ModalWindowDimBg", ImGuiCol_ModalWindowDimBg),
};

// Internal Load/Dump functions

static const std::string SchemaVersionLabel = "ImGuiThemeVersion";
static const std::string SchemaVersionValue = "1.0.0";

static const std::string ThemeNameLabel = "ThemeName";
static const std::string StyleSectionLabel = "style";
static const std::string ColorsSectionLabel = "colors";

static bool ThemeJsonFromStyle(json& aOutJson, const ImGuiStyle& aStyle)
{
    aOutJson = json{{SchemaVersionLabel, SchemaVersionValue}, {ThemeNameLabel, "<final merged theme in effect>"}, {StyleSectionLabel, {}}, {ColorsSectionLabel, {}}};

    json& styleJson = aOutJson[StyleSectionLabel];

    for (const auto& valueEncoder : StyleCodecs)
    {
        valueEncoder.encodeFrom(aStyle, styleJson);
    }

    json& colorJson = aOutJson[ColorsSectionLabel];

    for (const auto& valueEncoder : ColorCodecs)
    {
        valueEncoder.encodeFrom(aStyle, colorJson);
    }

    return true;
}

static bool StyleFromThemeJson(ImGuiStyle& aOutStyle, std::ifstream& aMaybeTheme)
{
    const json themeJson = json::parse(aMaybeTheme, nullptr, false, true);

    if (themeJson.is_discarded())
    {
        Log::Error("Failed to parse theme json");
        return false;
    }

    std::string schemaVersion = themeJson.value(SchemaVersionLabel, "");

    if (schemaVersion != SchemaVersionValue)
    {
        Log::Error("Theme schema version {} is not supported, may be invalid", schemaVersion);
        return false;
    }

    Log::Info("Successfully parsed theme: {}", themeJson.value(ThemeNameLabel, "<unnamed theme>"));
    Log::Info("Theme: {}", themeJson.dump(4));

    json styleParameters = themeJson.value(StyleSectionLabel, json::object());

    for (const auto& valueDecoder : StyleCodecs)
    {
        valueDecoder.decodeInto(aOutStyle, styleParameters);
    }

    json colorParameters = themeJson.value(ColorsSectionLabel, json::object());

    for (const auto& valueDecoder : ColorCodecs)
    {
        valueDecoder.decodeInto(aOutStyle, colorParameters);
    }

    return true;
}

//
// API
//

bool LoadStyleFromThemeJson(const std::filesystem::path& aThemePath, ImGuiStyle& aOutStyle)
{
    bool loadedSuccessfully = false;

    const auto cThemeJsonPath = GetAbsolutePath(aThemePath, "", false);

    Log::Info("Trying to load theme from {}...", UTF16ToUTF8(cThemeJsonPath.native()));

    if (!cThemeJsonPath.empty())
    {
        std::ifstream themeJson(cThemeJsonPath);
        if (!themeJson)
        {
            Log::Info("No theme file found!");
        }
        else
        {
            loadedSuccessfully = StyleFromThemeJson(aOutStyle, themeJson);
            if (!loadedSuccessfully)
                Log::Warn("Loading theme failed!");
        }
    }

    return loadedSuccessfully;
}

const std::string DumpStyleToThemeJson(const ImGuiStyle& aStyle)
{
    json themeJson;
    ThemeJsonFromStyle(themeJson, aStyle);

    return themeJson.dump(4);
}
