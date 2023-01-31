#pragma once

// "Theme" is a configurable set of CET ImGui styling parameters
//
// NB: this is not a serialization of ImGuiStyle, it's a
//     serializable representation of styling parameters.

// Load ImGui style directly from JSON. Parses the format used by DumpStyleToThemeJson.
bool LoadStyleFromThemeJson(const std::filesystem::path& aThemePath, ImGuiStyle& aOutStyle);

// Dump ImGui style into a JSON string with some extra metadata. Can be saved.
const std::string DumpStyleToThemeJson(const ImGuiStyle& aStyle);
