#pragma once

// "Theme" is a configurable set of CET ImGui styling parameters
//
// NB: this is not a serialization of ImGuiStyle, it's a
//     serializable representation of styling parameters.


// Dump ImGui style into JSON with some extra metadata
bool ThemeJsonFromStyle(nlohmann::json& aOutJson, const ImGuiStyle& aStyle);

// Load ImGui style from JSON
bool StyleFromThemeJson(ImGuiStyle& aOutTheme, std::ifstream& aMaybeTheme);
