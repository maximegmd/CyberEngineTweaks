#include "stdafx.h"

#include <fmt/core.h>
#include <scn/scn.h>

#include "Utils.h"

#include "Theme.h"


// Helpers


static ImVec2 XyFromJson(const nlohmann::json& aJson)
{
    return ImVec2(aJson[0], aJson[1]);
}

static nlohmann::json XyToJson(const ImVec2& aXy)
{
    return nlohmann::json::array({aXy.x, aXy.y});
}


static std::string ColorToHex(const ImVec4& aColor)
{
    //                      R     G     B     A
    return fmt::format("#{:02x}{:02x}{:02x}{:02x}",
                       static_cast<int>(aColor.x * 255),
                       static_cast<int>(aColor.y * 255),
                       static_cast<int>(aColor.z * 255),
                       static_cast<int>(aColor.w * 255));
}

static std::optional<ImVec4> HexToColor(const std::string& aHex)
{
    Log::Info("HTC: got {}", aHex);
    int r, g, b = 0;

    auto foundRGB = scn::scan(aHex, "#{:2x}{:2x}{:2x}", r, g, b);

    if (!foundRGB) {
        Log::Info("HTC: failed to parse");
        return std::nullopt;
    }

    int a = 255;
    auto alphaResult = scn::scan(foundRGB.range(), "{:2x}", a);

    Log::Info("HTC: parsed {} {} {} {}", r, g, b, a);
    ImVec4 outColor {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f
    };

    Log::Info("HTC: returning {}", ColorToHex(outColor));
    return outColor;
}


static bool ThemeJsonFromStyle(nlohmann::json& aOutJson, const ImGuiStyle& aStyle)
{
    aOutJson = nlohmann::ordered_json {
        {"cetThemeSchemaVersion", "1.0.0"},
        {"cetThemeName", "<final merged theme in effect>"},
        {"style", {
            {"Alpha", aStyle.Alpha},
            {"AntiAliasedLines", aStyle.AntiAliasedLines},
            {"AntiAliasedFill", aStyle.AntiAliasedFill},
            {"DisplayWindowPadding", XyToJson(aStyle.DisplayWindowPadding)},
            {"DisplaySafeAreaPadding", XyToJson(aStyle.DisplaySafeAreaPadding)},
            {"WindowRounding", aStyle.WindowRounding},
            {"WindowBorderSize", aStyle.WindowBorderSize},
            {"WindowMinSize", XyToJson(aStyle.WindowMinSize)},
            {"WindowTitleAlign", XyToJson(aStyle.WindowTitleAlign)},
            {"ChildRounding", aStyle.ChildRounding},
            {"ChildBorderSize", aStyle.ChildBorderSize},
            {"PopupRounding", aStyle.PopupRounding},
            {"PopupBorderSize", aStyle.PopupBorderSize},
            {"FramePadding", XyToJson(aStyle.FramePadding)},
            {"FrameRounding", aStyle.FrameRounding},
            {"FrameBorderSize", aStyle.FrameBorderSize},
            {"ItemSpacing", XyToJson(aStyle.ItemSpacing)},
            {"ItemInnerSpacing", XyToJson(aStyle.ItemInnerSpacing)},
            {"TouchExtraPadding", XyToJson(aStyle.TouchExtraPadding)},
            {"IndentSpacing", aStyle.IndentSpacing},
            {"ColumnsMinSpacing", aStyle.ColumnsMinSpacing},
            {"ScrollbarSize", aStyle.ScrollbarSize},
            {"ScrollbarRounding", aStyle.ScrollbarRounding},
            {"GrabMinSize", aStyle.GrabMinSize},
            {"GrabRounding", aStyle.GrabRounding},
            {"TabRounding", aStyle.TabRounding},
            {"TabBorderSize", aStyle.TabBorderSize},
            {"TabMinWidthForCloseButton", aStyle.TabMinWidthForCloseButton},
            {"ColorButtonPosition", aStyle.ColorButtonPosition},
            {"ButtonTextAlign", XyToJson(aStyle.ButtonTextAlign)},
            {"SelectableTextAlign", XyToJson(aStyle.SelectableTextAlign)},
            {"DisplayWindowPadding", XyToJson(aStyle.DisplayWindowPadding)},
            {"DisplaySafeAreaPadding", XyToJson(aStyle.DisplaySafeAreaPadding)},
            {"MouseCursorScale", aStyle.MouseCursorScale},
            {"CurveTessellationTol", aStyle.CurveTessellationTol}
        }},
        {"colors", {
            {"Text", ColorToHex(aStyle.Colors[ImGuiCol_Text])},
            {"TextDisabled", ColorToHex(aStyle.Colors[ImGuiCol_TextDisabled])},
            {"WindowBg", ColorToHex(aStyle.Colors[ImGuiCol_WindowBg])},
            {"ChildBg", ColorToHex(aStyle.Colors[ImGuiCol_ChildBg])},
            {"PopupBg", ColorToHex(aStyle.Colors[ImGuiCol_PopupBg])},
            {"Border", ColorToHex(aStyle.Colors[ImGuiCol_Border])},
            {"BorderShadow", ColorToHex(aStyle.Colors[ImGuiCol_BorderShadow])},
            {"FrameBg", ColorToHex(aStyle.Colors[ImGuiCol_FrameBg])},
            {"FrameBgHovered", ColorToHex(aStyle.Colors[ImGuiCol_FrameBgHovered])},
            {"FrameBgActive", ColorToHex(aStyle.Colors[ImGuiCol_FrameBgActive])},
            {"TitleBg", ColorToHex(aStyle.Colors[ImGuiCol_TitleBg])},
            {"TitleBgActive", ColorToHex(aStyle.Colors[ImGuiCol_TitleBgActive])},
            {"TitleBgCollapsed", ColorToHex(aStyle.Colors[ImGuiCol_TitleBgCollapsed])},
            {"MenuBarBg", ColorToHex(aStyle.Colors[ImGuiCol_MenuBarBg])},
            {"ScrollbarBg", ColorToHex(aStyle.Colors[ImGuiCol_ScrollbarBg])},
            {"ScrollbarGrab", ColorToHex(aStyle.Colors[ImGuiCol_ScrollbarGrab])},
            {"ScrollbarGrabHovered", ColorToHex(aStyle.Colors[ImGuiCol_ScrollbarGrabHovered])},
            {"ScrollbarGrabActive", ColorToHex(aStyle.Colors[ImGuiCol_ScrollbarGrabActive])},
            {"CheckMark", ColorToHex(aStyle.Colors[ImGuiCol_CheckMark])},
            {"SliderGrab", ColorToHex(aStyle.Colors[ImGuiCol_SliderGrab])},
            {"SliderGrabActive", ColorToHex(aStyle.Colors[ImGuiCol_SliderGrabActive])},
            {"Button", ColorToHex(aStyle.Colors[ImGuiCol_Button])},
            {"ButtonHovered", ColorToHex(aStyle.Colors[ImGuiCol_ButtonHovered])},
            {"ButtonActive", ColorToHex(aStyle.Colors[ImGuiCol_ButtonActive])},
            {"Header", ColorToHex(aStyle.Colors[ImGuiCol_Header])},
            {"HeaderHovered", ColorToHex(aStyle.Colors[ImGuiCol_HeaderHovered])},
            {"HeaderActive", ColorToHex(aStyle.Colors[ImGuiCol_HeaderActive])},
            {"Separator", ColorToHex(aStyle.Colors[ImGuiCol_Separator])},
            {"SeparatorHovered", ColorToHex(aStyle.Colors[ImGuiCol_SeparatorHovered])},
            {"SeparatorActive", ColorToHex(aStyle.Colors[ImGuiCol_SeparatorActive])},
            {"ResizeGrip", ColorToHex(aStyle.Colors[ImGuiCol_ResizeGrip])},
            {"ResizeGripHovered", ColorToHex(aStyle.Colors[ImGuiCol_ResizeGripHovered])},
            {"ResizeGripActive", ColorToHex(aStyle.Colors[ImGuiCol_ResizeGripActive])},
            {"Tab", ColorToHex(aStyle.Colors[ImGuiCol_Tab])},
            {"TabHovered", ColorToHex(aStyle.Colors[ImGuiCol_TabHovered])},
            {"TabActive", ColorToHex(aStyle.Colors[ImGuiCol_TabActive])},
            {"TabUnfocused", ColorToHex(aStyle.Colors[ImGuiCol_TabUnfocused])},
            {"TabUnfocusedActive", ColorToHex(aStyle.Colors[ImGuiCol_TabUnfocusedActive])},
            {"PlotLines", ColorToHex(aStyle.Colors[ImGuiCol_PlotLines])},
            {"PlotLinesHovered", ColorToHex(aStyle.Colors[ImGuiCol_PlotLinesHovered])},
            {"PlotHistogram", ColorToHex(aStyle.Colors[ImGuiCol_PlotHistogram])},
            {"PlotHistogramHovered", ColorToHex(aStyle.Colors[ImGuiCol_PlotHistogramHovered])},
            {"TextSelectedBg", ColorToHex(aStyle.Colors[ImGuiCol_TextSelectedBg])},
            {"DragDropTarget", ColorToHex(aStyle.Colors[ImGuiCol_DragDropTarget])},
            {"NavHighlight", ColorToHex(aStyle.Colors[ImGuiCol_NavHighlight])},
            {"NavWindowingHighlight", ColorToHex(aStyle.Colors[ImGuiCol_NavWindowingHighlight])},
            {"NavWindowingDimBg", ColorToHex(aStyle.Colors[ImGuiCol_NavWindowingDimBg])},
            {"ModalWindowDimBg", ColorToHex(aStyle.Colors[ImGuiCol_ModalWindowDimBg])},
            {"DockingPreview", ColorToHex(aStyle.Colors[ImGuiCol_DockingPreview])}
        }}
    };

    return true;
}



static bool StyleFromThemeJson(ImGuiStyle& aOutTheme, std::ifstream& aMaybeTheme)
{
    const nlohmann::json themeJson = nlohmann::json::parse(aMaybeTheme, nullptr, false, true);

    if (themeJson.is_discarded()) {
        Log::Error("Failed to parse theme json");
        return false;
    }

    auto schemaVersion = themeJson.value("cetThemeSchemaVersion", "");

    if (schemaVersion != "1.0.0") {
        Log::Error("Theme schema version {} is not supported, may be invalid", schemaVersion);
        return false;
    }

    Log::Info("Successfully loaded theme: {}", themeJson.dump(4));

    auto styleParameters = themeJson.value("style", nlohmann::json::object());

    aOutTheme.Alpha = styleParameters.value("Alpha", aOutTheme.Alpha);
    aOutTheme.AntiAliasedLines = styleParameters.value("AntiAliasedLines", aOutTheme.AntiAliasedLines);
    aOutTheme.AntiAliasedFill = styleParameters.value("AntiAliasedFill", aOutTheme.AntiAliasedFill);
    auto windowPadding = styleParameters.value("WindowPadding", nlohmann::json::array({aOutTheme.WindowPadding.x, aOutTheme.WindowPadding.y}));
    aOutTheme.WindowPadding = ImVec2(windowPadding[0], windowPadding[1]);

    auto colorParameters = themeJson.value("colors", nlohmann::json::object());

    aOutTheme.Colors[ImGuiCol_Text] = HexToColor(colorParameters.value("Text", "")).value_or(aOutTheme.Colors[ImGuiCol_Text]);
    aOutTheme.Colors[ImGuiCol_WindowBg] = HexToColor(colorParameters.value("WindowBg", "")).value_or(aOutTheme.Colors[ImGuiCol_WindowBg]);

    return true;
}


// API


bool LoadStyleFromThemeJson(const std::filesystem::path& aThemePath, ImGuiStyle& aOutStyle)
{
    bool loaded = false;

    const auto cThemeJsonPath = GetAbsolutePath(aThemePath, "", false);

    Log::Info("Trying to load theme from {}...", UTF16ToUTF8(cThemeJsonPath.native()));

    if (!cThemeJsonPath.empty())
    {
        std::ifstream themeJson(cThemeJsonPath);
        if(!themeJson)
        {
            Log::Info("No theme file found!");
        }
        else
        {
            loaded = StyleFromThemeJson(aOutStyle, themeJson);
            if (!loaded)
                Log::Warn("Loading theme failed!");
        }

        themeJson.close();
    }

    return loaded;
}


const std::string DumpStyleToThemeJson(const ImGuiStyle& aStyle)
{
    nlohmann::json themeJson;
    ThemeJsonFromStyle(themeJson, aStyle);

    return themeJson.dump(4);
}
