#include "stdafx.h"

#include "Theme.h"

static ImVec4 ColorFromJson(const nlohmann::json& aJson)
{
    return ImVec4(aJson[0], aJson[1], aJson[2], aJson[3]);
}

static nlohmann::json ColorToJson(const ImVec4& aColor)
{
    return nlohmann::json::array({aColor.x, aColor.y, aColor.z, aColor.w});
}

/*
            static xxx color to obj
            obj to color
            imvec to obj/ary
            ary to imvec
            */
bool ThemeJsonFromStyle(nlohmann::json& aOutJson, const ImGuiStyle& aStyle)
{
    aOutJson = nlohmann::json {
        {"cetThemeSchemaVersion", "1.0.0"},
        {"cetThemeName", "<final merged theme in effect>"},
        {"style",
            {
                {"Alpha", aStyle.Alpha},
                {"AntiAliasedLines", aStyle.AntiAliasedLines},
                {"AntiAliasedFill", aStyle.AntiAliasedFill},
                {"DisplayWindowPadding", {aStyle.DisplayWindowPadding.x, aStyle.DisplayWindowPadding.y}},
                {"DisplaySafeAreaPadding", {aStyle.DisplaySafeAreaPadding.x, aStyle.DisplaySafeAreaPadding.y}},
                {"FramePadding", {aStyle.FramePadding.x, aStyle.FramePadding.y}},
                {"FrameRounding", aStyle.FrameRounding},
                {"FrameBorderSize", aStyle.FrameBorderSize},
                {"WindowTitleAlign", {aStyle.WindowTitleAlign.x, aStyle.WindowTitleAlign.y}},
                {"WindowPadding", {aStyle.WindowPadding.x, aStyle.WindowPadding.y}},
                {"WindowRounding", aStyle.WindowRounding},
                {"WindowBorderSize", aStyle.WindowBorderSize},
                {"WindowMinSize", {aStyle.WindowMinSize.x, aStyle.WindowMinSize.y}},
                {"ChildBorderSize", aStyle.ChildBorderSize},
                {"ChildRounding", aStyle.ChildRounding},
                {"PopupRounding", aStyle.PopupRounding},
                {"PopupBorderSize", aStyle.PopupBorderSize},
                {"TabRounding", aStyle.TabRounding},
                {"TabBorderSize", aStyle.TabBorderSize},
                {"TabMinWidthForCloseButton", aStyle.TabMinWidthForCloseButton},
                {"ColumnsMinSpacing", aStyle.ColumnsMinSpacing},
                {"CellPadding", {aStyle.CellPadding.x, aStyle.CellPadding.y}},
                {"ItemSpacing", {aStyle.ItemSpacing.x, aStyle.ItemSpacing.y}},
                {"ItemInnerSpacing", {aStyle.ItemInnerSpacing.x, aStyle.ItemInnerSpacing.y}},
                {"TouchExtraPadding", {aStyle.TouchExtraPadding.x, aStyle.TouchExtraPadding.y}},
                {"ScrollbarSize", aStyle.ScrollbarSize},
                {"ScrollbarRounding", aStyle.ScrollbarRounding},
                {"GrabRounding", aStyle.GrabRounding},
                {"GrabMinSize", aStyle.GrabMinSize},
                {"ColorButtonPosition", aStyle.ColorButtonPosition},
                {"IndentSpacing", aStyle.IndentSpacing},
                {"ButtonTextAlign", {aStyle.ButtonTextAlign.x, aStyle.ButtonTextAlign.y}},
                {"SelectableTextAlign", {aStyle.SelectableTextAlign.x, aStyle.SelectableTextAlign.y}},
                {"MouseCursorScale", aStyle.MouseCursorScale},
                {"CurveTessellationTol", aStyle.CurveTessellationTol},
            }
        }
    };
            /*
            static xxx color to obj
            obj to color
            imvec to obj/ary
            ary to imvec
        },
        {"colors",
            {"Text", {aStyle.Colors[ImGuiCol_Text].x, aStyle.Colors[ImGuiCol_Text].y, aStyle.Colors[ImGuiCol_Text].z, aStyle.Colors[ImGuiCol_Text].w}},
            {"TextDisabled", {aStyle.Colors[ImGuiCol_TextDisabled].x, aStyle.Colors[ImGuiCol_TextDisabled].y, aStyle.Colors[ImGuiCol_TextDisabled].z, aStyle.Colors[ImGuiCol_TextDisabled].w}},
            {"WindowBg", {aStyle.Colors[ImGuiCol_WindowBg].x, aStyle.Colors[ImGuiCol_WindowBg].y, aStyle.Colors[ImGuiCol_WindowBg].z, aStyle.Colors[ImGuiCol_WindowBg].w}},
            {"ChildBg", {aStyle.Colors[ImGuiCol_ChildBg].x, aStyle.Colors[ImGuiCol_ChildBg].y, aStyle.Colors[ImGuiCol_ChildBg].z, aStyle.Colors[ImGuiCol_ChildBg].w}},
            {"PopupBg", {aStyle.Colors[ImGuiCol_PopupBg].x, aStyle.Colors[ImGuiCol_PopupBg].y, aStyle.Colors[ImGuiCol_PopupBg].z, aStyle.Colors[ImGuiCol_PopupBg].w}},
            {"Border", {aStyle.Colors[ImGuiCol_Border].x, aStyle.Colors[ImGuiCol_Border].y, aStyle.Colors[ImGuiCol_Border].z, aStyle.Colors[ImGuiCol_Border].w}},
            {"BorderShadow", {aStyle.Colors[ImGuiCol_BorderShadow].x, aStyle.Colors[ImGuiCol_BorderShadow].y, aStyle.Colors[ImGuiCol_BorderShadow].z, aStyle.Colors[ImGuiCol_BorderShadow].w}},
            {"FrameBg", {aStyle.Colors[ImGuiCol_FrameBg].x, aStyle.Colors[ImGuiCol_FrameBg].y, aStyle.Colors[ImGuiCol_FrameBg].z, aStyle.Colors[ImGuiCol_FrameBg].w}},
            {"FrameBgHovered", {aStyle.Colors[ImGuiCol_FrameBgHovered].x, aStyle.Colors[ImGuiCol_FrameBgHovered].y, aStyle.Colors[ImGuiCol_FrameBgHovered].z, aStyle.Colors[ImGuiCol_FrameBgHovered].w}},
            {"FrameBgActive", {aStyle.Colors[ImGuiCol_FrameBgActive].x, aStyle.Colors[ImGuiCol_FrameBgActive].y, aStyle.Colors[ImGuiCol_FrameBgActive].z, aStyle.Colors[ImGuiCol_FrameBgActive].w}},
            {"TitleBg", {aStyle.Colors[ImGuiCol_TitleBg].x, aStyle.Colors[ImGuiCol_TitleBg].y, aStyle.Colors[ImGuiCol_TitleBg].z, aStyle.Colors[ImGuiCol_TitleBg].w}},
            {"TitleBgActive", {aStyle.Colors[ImGuiCol_TitleBgActive].x, aStyle.Colors[ImGuiCol_TitleBgActive].y, aStyle.Colors[ImGuiCol_TitleBgActive].z, aStyle.Colors[ImGuiCol_TitleBgActive].w}},
            {"TitleBgCollapsed", {aStyle.Colors[ImGuiCol_TitleBgCollapsed].x, aStyle.Colors[ImGuiCol_TitleBgCollapsed].y, aStyle.Colors[ImGuiCol_TitleBgCollapsed].z, aStyle.Colors[ImGuiCol_TitleBgCollapsed].w}},
            {"MenuBarBg", {aStyle.Colors[ImGuiCol_MenuBarBg].x, aStyle.Colors[ImGuiCol_MenuBarBg].y, aStyle.Colors[ImGuiCol_MenuBarBg].z, aStyle.Colors[ImGuiCol_MenuBarBg].w}},
            {"ScrollbarBg", {aStyle.Colors[ImGuiCol_ScrollbarBg].x, aStyle.Colors[ImGuiCol_ScrollbarBg].y, aStyle.Colors[ImGuiCol_ScrollbarBg].z, aStyle.Colors[ImGuiCol_ScrollbarBg].w}},
            {"ScrollbarGrab", {aStyle.Colors[ImGuiCol_ScrollbarGrab].x, aStyle.Colors[ImGuiCol_ScrollbarGrab].y, aStyle.Colors[ImGuiCol_ScrollbarGrab].z, aStyle.Colors[ImGuiCol_ScrollbarGrab].w}},
            {"ScrollbarGrabHovered", {aStyle.Colors[ImGuiCol_ScrollbarGrabHovered].x, aStyle.Colors[ImGuiCol_ScrollbarGrabHovered].y, aStyle.Colors[ImGuiCol_ScrollbarGrabHovered].z, aStyle.Colors[ImGuiCol_ScrollbarGrabHovered].w}},
            {"ScrollbarGrabActive", {aStyle.Colors[ImGuiCol_ScrollbarGrabActive].x, aStyle.Colors[ImGuiCol_ScrollbarGrabActive].y, aStyle.Colors[ImGuiCol_ScrollbarGrabActive].z, aStyle.Colors[ImGuiCol_ScrollbarGrabActive].w}},
            {"CheckMark", {aStyle.Colors[ImGuiCol_CheckMark].x, aStyle.Colors[ImGuiCol_CheckMark].y, aStyle.Colors[ImGuiCol_CheckMark].z, aStyle.Colors[ImGuiCol_CheckMark].w}},
            {"SliderGrab", {aStyle.Colors[ImGuiCol_SliderGrab].x, aStyle.Colors[ImGuiCol_SliderGrab].y, aStyle.Colors[ImGuiCol_SliderGrab].z, aStyle.Colors[ImGuiCol_SliderGrab].w}},
            {"SliderGrabActive", {aStyle.Colors[ImGuiCol_SliderGrabActive].x, aStyle.Colors[ImGuiCol_SliderGrabActive].y, aStyle.Colors[ImGuiCol_SliderGrabActive].z, aStyle.Colors[ImGuiCol_SliderGrabActive].w}},
            {"Button", {aStyle.Colors[ImGuiCol_Button].x, aStyle.Colors[ImGuiCol_Button].y, aStyle.Colors[ImGuiCol_Button].z, aStyle.Colors[ImGuiCol_Button].w}},
            {"ButtonHovered", {aStyle.Colors[ImGuiCol_ButtonHovered].x, aStyle.Colors[ImGuiCol_ButtonHovered].y, aStyle.Colors[ImGuiCol_ButtonHovered].z, aStyle.Colors[ImGuiCol_ButtonHovered].w}},
            {"ButtonActive", {aStyle.Colors[ImGuiCol_ButtonActive].x, aStyle.Colors[ImGuiCol_ButtonActive].y, aStyle.Colors[ImGuiCol_ButtonActive].z, aStyle.Colors[ImGuiCol_ButtonActive].w}},
            {"Header", {aStyle.Colors[ImGuiCol_Header].x, aStyle.Colors[ImGuiCol_Header].y, aStyle.Colors[ImGuiCol_Header].z, aStyle.Colors[ImGuiCol_Header].w}},
            {"HeaderHovered", {aStyle.Colors[ImGuiCol_HeaderHovered].x, aStyle.Colors[ImGuiCol_HeaderHovered].y, aStyle.Colors[ImGuiCol_HeaderHovered].z, aStyle.Colors[ImGuiCol_HeaderHovered].w}},
            {"HeaderActive", {aStyle.Colors[ImGuiCol_HeaderActive].x, aStyle.Colors[ImGuiCol_HeaderActive].y, aStyle.Colors[ImGuiCol_HeaderActive].z, aStyle.Colors[ImGuiCol_HeaderActive].w}},
            {"Separator", {aStyle.Colors[ImGuiCol_Separator].x, aStyle.Colors[ImGuiCol_Separator].y, aStyle.Colors[ImGuiCol_Separator].z, aStyle.Colors[ImGuiCol_Separator].w}},
            {"SeparatorHovered", {aStyle.Colors[ImGuiCol_SeparatorHovered].x, aStyle.Colors[ImGuiCol_SeparatorHovered].y, aStyle.Colors[ImGuiCol_SeparatorHovered].z, aStyle.Colors[ImGuiCol_SeparatorHovered].w}},
            {"SeparatorActive", {aStyle.Colors[ImGuiCol_SeparatorActive].x, aStyle.Colors[ImGuiCol_SeparatorActive].y, aStyle.Colors[ImGuiCol_SeparatorActive].z, aStyle.Colors[ImGuiCol_SeparatorActive].w}},
            {"ResizeGrip", {aStyle.Colors[ImGuiCol_ResizeGrip].x, aStyle.Colors[ImGuiCol_ResizeGrip].y, aStyle.Colors[ImGuiCol_ResizeGrip].z, aStyle.Colors[ImGuiCol_ResizeGrip].w}},
            {"ResizeGripHovered", {aStyle.Colors[ImGuiCol_ResizeGripHovered].x, aStyle.Colors[ImGuiCol_ResizeGripHovered].y, aStyle.Colors[ImGuiCol_ResizeGripHovered].z, aStyle.Colors[ImGuiCol_ResizeGripHovered].w}},
            {"ResizeGripActive", {aStyle.Colors[ImGuiCol_ResizeGripActive].x, aStyle.Colors[ImGuiCol_ResizeGripActive].y, aStyle.Colors[ImGuiCol_ResizeGripActive].z, aStyle.Colors[ImGuiCol_ResizeGripActive].w}},
            {"Tab", {aStyle.Colors[ImGuiCol_Tab].x, aStyle.Colors[ImGuiCol_Tab].y, aStyle.Colors[ImGuiCol_Tab].z, aStyle.Colors[ImGuiCol_Tab].w}},
            {"TabHovered", {aStyle.Colors[ImGuiCol_TabHovered].x, aStyle.Colors[ImGuiCol_TabHovered].y, aStyle.Colors[ImGuiCol_TabHovered].z, aStyle.Colors[ImGuiCol_TabHovered].w}},
            {"TabActive", {aStyle.Colors[ImGuiCol_TabActive].x, aStyle.Colors[ImGuiCol_TabActive].y, aStyle.Colors[ImGuiCol_TabActive].z, aStyle.Colors[ImGuiCol_TabActive].w}},
            {"TabUnfocused", {aStyle.Colors[ImGuiCol_TabUnfocused].x, aStyle.Colors[ImGuiCol_TabUnfocused].y, aStyle.Colors[ImGuiCol_TabUnfocused].z, aStyle.Colors[ImGuiCol_TabUnfocused].w}},
            {"TabUnfocusedActive", {aStyle.Colors[ImGuiCol_TabUnfocusedActive].x, aStyle.Colors[ImGuiCol_TabUnfocusedActive].y, aStyle.Colors[ImGuiCol_TabUnfocusedActive].z, aStyle.Colors[ImGuiCol_TabUnfocusedActive].w}},
            {"PlotLines", {aStyle.Colors[ImGuiCol_PlotLines].x, aStyle.Colors[ImGuiCol_PlotLines].y, aStyle.Colors[ImGuiCol_PlotLines].z, aStyle.Colors[ImGuiCol_PlotLines].w}},
            {"PlotLinesHovered", {aStyle.Colors[ImGuiCol_PlotLinesHovered].x, aStyle.Colors[ImGuiCol_PlotLinesHovered].y, aStyle.Colors[ImGuiCol_PlotLinesHovered].z, aStyle.Colors[ImGuiCol_PlotLinesHovered].w}},
            {"PlotHistogram", {aStyle.Colors[ImGuiCol_PlotHistogram].x, aStyle.Colors[ImGuiCol_PlotHistogram].y, aStyle.Colors[ImGuiCol_PlotHistogram].z, aStyle.Colors[ImGuiCol_PlotHistogram].w}},
            {"PlotHistogramHovered", {aStyle.Colors[ImGuiCol_PlotHistogramHovered].x, aStyle.Colors[ImGuiCol_PlotHistogramHovered].y, aStyle.Colors[ImGuiCol_PlotHistogramHovered].z, aStyle.Colors[ImGuiCol_PlotHistogramHovered].w}},
            {"TextSelectedBg", {aStyle.Colors[ImGuiCol_TextSelectedBg].x, aStyle.Colors[ImGuiCol_TextSelectedBg].y, aStyle.Colors[ImGuiCol_TextSelectedBg].z, aStyle.Colors[ImGuiCol_TextSelectedBg].w}},
            {"DragDropTarget", {aStyle.Colors[ImGuiCol_DragDropTarget].x, aStyle.Colors[ImGuiCol_DragDropTarget].y, aStyle.Colors[ImGuiCol_DragDropTarget].z, aStyle.Colors[ImGuiCol_DragDropTarget].w}},
            {"NavHighlight", {aStyle.Colors[ImGuiCol_NavHighlight].x, aStyle.Colors[ImGuiCol_NavHighlight].y, aStyle.Colors[ImGuiCol_NavHighlight].z, aStyle.Colors[ImGuiCol_NavHighlight].w}},
            {"NavWindowingHighlight", {aStyle.Colors[ImGuiCol_NavWindowingHighlight].x, aStyle.Colors[ImGuiCol_NavWindowingHighlight].y, aStyle.Colors[ImGuiCol_NavWindowingHighlight].z, aStyle.Colors[ImGuiCol_NavWindowingHighlight].w}},
            {"NavWindowingDimBg", {aStyle.Colors[ImGuiCol_NavWindowingDimBg].x, aStyle.Colors[ImGuiCol_NavWindowingDimBg].y, aStyle.Colors[ImGuiCol_NavWindowingDimBg].z, aStyle.Colors[ImGuiCol_NavWindowingDimBg].w}},
            {"ModalWindowDimBg", {aStyle.Colors[ImGuiCol_ModalWindowDimBg].x, aStyle.Colors[ImGuiCol_ModalWindowDimBg].y, aStyle.Colors[ImGuiCol_ModalWindowDimBg].z, aStyle.Colors[ImGuiCol_ModalWindowDimBg].w}},
            {"DockingPreview", {aStyle.Colors[ImGuiCol_DockingPreview].x, aStyle.Colors[ImGuiCol_DockingPreview].y, aStyle.Colors[ImGuiCol_DockingPreview].z, aStyle.Colors[ImGuiCol_DockingPreview].w}},
            {"DockingEmptyBg", {aStyle.Colors[ImGuiCol_DockingEmptyBg].x, aStyle.Colors[ImGuiCol_DockingEmptyBg].y, aStyle.Colors[ImGuiCol_DockingEmptyBg].z, aStyle.Colors[ImGuiCol_DockingEmptyBg].w}},
            {"TableHeaderBg", {aStyle.Colors[ImGuiCol_TableHeaderBg].x, aStyle.Colors[ImGuiCol_TableHeaderBg].y, aStyle.Colors[ImGuiCol_TableHeaderBg].z, aStyle.Colors[ImGuiCol_TableHeaderBg].w}},
            {"TableBorderStrong", {aStyle.Colors[ImGuiCol_TableBorderStrong].x, aStyle.Colors[ImGuiCol_TableBorderStrong].y, aStyle.Colors[ImGuiCol_TableBorderStrong].z, aStyle.Colors[ImGuiCol_TableBorderStrong].w}},
            {"TableBorderLight", {aStyle.Colors[ImGuiCol_TableBorderLight].x, aStyle.Colors[ImGuiCol_TableBorderLight].y, aStyle.Colors[ImGuiCol_TableBorderLight].z, aStyle.Colors[ImGuiCol_TableBorderLight].w}},
            {"TableRowBg", {aStyle.Colors[ImGuiCol_TableRowBg].x, aStyle.Colors[ImGuiCol_TableRowBg].y, aStyle.Colors[ImGuiCol_TableRowBg].z, aStyle.Colors[ImGuiCol_TableRowBg].w}},
        }
        */

    return true;
}

bool StyleFromThemeJson(ImGuiStyle& aOutTheme, std::ifstream& aMaybeTheme)
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

    return true;
}