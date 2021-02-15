#pragma once

namespace sol_ImGui
{
    // Windows
    inline bool Begin(const std::string& name)                              { return ImGui::Begin(name.c_str()); }
    inline bool Begin(const std::string& name, int flags)                   { return ImGui::Begin(name.c_str(), NULL, static_cast<ImGuiWindowFlags_>(flags)); }
    inline std::tuple<bool, bool> Begin(const std::string& name, bool open)
    {
        if (!open) return std::make_tuple(false, false);
        const bool shouldDraw = ImGui::Begin(name.c_str(), &open);
        return std::make_tuple(open, open && shouldDraw);
    }
    inline std::tuple<bool, bool> Begin(const std::string& name, bool open, int flags)
    {
        if (!open) return std::make_tuple(false, false);
        const bool shouldDraw = ImGui::Begin(name.c_str(), &open, static_cast<ImGuiWindowFlags_>(flags));
        return std::make_tuple(open, open && shouldDraw);
    }
    inline void End()                                               { ImGui::End(); }

    // Child Windows
    inline bool BeginChild(const std::string& name)                                                       { return ImGui::BeginChild(name.c_str()); }
    inline bool BeginChild(const std::string& name, float sizeX)                                          { return ImGui::BeginChild(name.c_str(), { sizeX, 0 }); }
    inline bool BeginChild(const std::string& name, float sizeX, float sizeY)                             { return ImGui::BeginChild(name.c_str(), { sizeX, sizeY }); }
    inline bool BeginChild(const std::string& name, float sizeX, float sizeY, bool border)                { return ImGui::BeginChild(name.c_str(), { sizeX, sizeY }, border); }
    inline bool BeginChild(const std::string& name, float sizeX, float sizeY, bool border, int flags)     { return ImGui::BeginChild(name.c_str(), { sizeX, sizeY }, border, static_cast<ImGuiWindowFlags>(flags)); }
    inline void EndChild()                                                                                { ImGui::EndChild(); }

    // Windows Utilities
    inline bool IsWindowAppearing()                                  { return ImGui::IsWindowAppearing(); }
    inline bool IsWindowCollapsed()                                  { return ImGui::IsWindowCollapsed(); }
    inline bool IsWindowFocused()                                    { return ImGui::IsWindowFocused(); }
    inline bool IsWindowFocused(int flags)                           { return ImGui::IsWindowFocused(static_cast<ImGuiFocusedFlags>(flags)); }
    inline bool IsWindowHovered()                                    { return ImGui::IsWindowHovered(); }
    inline bool IsWindowHovered(int flags)                           { return ImGui::IsWindowHovered(static_cast<ImGuiHoveredFlags>(flags)); }
    inline ImDrawList* GetWindowDrawList()                           { return ImGui::GetWindowDrawList(); }
    inline std::tuple<float, float> GetWindowPos()                   { const auto vec2{ ImGui::GetWindowPos() };  return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetWindowSize()                  { const auto vec2{ ImGui::GetWindowSize() };  return std::make_tuple(vec2.x, vec2.y); }
    inline float GetWindowWidth()                                    { return ImGui::GetWindowWidth(); }
    inline float GetWindowHeight()                                   { return ImGui::GetWindowHeight(); }

    // Prefer using SetNext...
    inline void SetNextWindowPos(float posX, float posY)                                              { ImGui::SetNextWindowPos({ posX, posY }); }
    inline void SetNextWindowPos(float posX, float posY, int cond)                                    { ImGui::SetNextWindowPos({ posX, posY }, static_cast<ImGuiCond>(cond)); }
    inline void SetNextWindowPos(float posX, float posY, int cond, float pivotX, float pivotY)        { ImGui::SetNextWindowPos({ posX, posY }, static_cast<ImGuiCond>(cond), { pivotX, pivotY }); }
    inline void SetNextWindowSize(float sizeX, float sizeY)                                           { ImGui::SetNextWindowSize({ sizeX, sizeY }); }
    inline void SetNextWindowSize(float sizeX, float sizeY, int cond)                                 { ImGui::SetNextWindowSize({ sizeX, sizeY }, static_cast<ImGuiCond>(cond)); }
    inline void SetNextWindowSizeConstraints(float minX, float minY, float maxX, float maxY)          { ImGui::SetNextWindowSizeConstraints({ minX, minY }, { maxX, maxY }); }
    inline void SetNextWindowContentSize(float sizeX, float sizeY)                                    { ImGui::SetNextWindowContentSize({ sizeX, sizeY }); }
    inline void SetNextWindowCollapsed(bool collapsed)                                                { ImGui::SetNextWindowCollapsed(collapsed); }
    inline void SetNextWindowCollapsed(bool collapsed, int cond)                                      { ImGui::SetNextWindowCollapsed(collapsed, static_cast<ImGuiCond>(cond)); }
    inline void SetNextWindowFocus()                                                                  { ImGui::SetNextWindowFocus(); }
    inline void SetNextWindowBgAlpha(float alpha)                                                     { ImGui::SetNextWindowBgAlpha(alpha); }
    inline void SetWindowPos(float posX, float posY)                                                  { ImGui::SetWindowPos({ posX, posY }); }
    inline void SetWindowPos(float posX, float posY, int cond)                                        { ImGui::SetWindowPos({ posX, posY }, static_cast<ImGuiCond>(cond)); }
    inline void SetWindowSize(float sizeX, float sizeY)                                               { ImGui::SetWindowSize({ sizeX, sizeY }); }
    inline void SetWindowSize(float sizeX, float sizeY, int cond)                                     { ImGui::SetWindowSize({ sizeX, sizeY }, static_cast<ImGuiCond>(cond)); }
    inline void SetWindowCollapsed(bool collapsed)                                                    { ImGui::SetWindowCollapsed(collapsed); }
    inline void SetWindowCollapsed(bool collapsed, int cond)                                          { ImGui::SetWindowCollapsed(collapsed, static_cast<ImGuiCond>(cond)); }
    inline void SetWindowFocus()                                                                      { ImGui::SetWindowFocus(); }
    inline void SetWindowFontScale(float scale)                                                       { ImGui::SetWindowFontScale(scale); }
    inline void SetWindowPos(const std::string& name, float posX, float posY)                         { ImGui::SetWindowPos(name.c_str(), { posX, posY }); }
    inline void SetWindowPos(const std::string& name, float posX, float posY, int cond)               { ImGui::SetWindowPos(name.c_str(), { posX, posY }, static_cast<ImGuiCond>(cond)); }
    inline void SetWindowSize(const std::string& name, float sizeX, float sizeY)                      { ImGui::SetWindowSize(name.c_str(), { sizeX, sizeY }); }
    inline void SetWindowSize(const std::string& name, float sizeX, float sizeY, int cond)            { ImGui::SetWindowSize(name.c_str(), { sizeX, sizeY }, static_cast<ImGuiCond>(cond)); }
    inline void SetWindowCollapsed(const std::string& name, bool collapsed)                           { ImGui::SetWindowCollapsed(name.c_str(), collapsed); }
    inline void SetWindowCollapsed(const std::string& name, bool collapsed, int cond)                 { ImGui::SetWindowCollapsed(name.c_str(), collapsed, static_cast<ImGuiCond>(cond)); }
    inline void SetWindowFocus(const std::string& name)                                               { ImGui::SetWindowFocus(name.c_str()); }

    // Content Region
    inline std::tuple<float, float> GetContentRegionMax()                            { const auto vec2{ ImGui::GetContentRegionMax() };  return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetContentRegionAvail()                          { const auto vec2{ ImGui::GetContentRegionAvail() };  return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetWindowContentRegionMin()                      { const auto vec2{ ImGui::GetWindowContentRegionMin() };  return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetWindowContentRegionMax()                      { const auto vec2{ ImGui::GetWindowContentRegionMax() };  return std::make_tuple(vec2.x, vec2.y); }
    inline float GetWindowContentRegionWidth()                                       { return ImGui::GetWindowContentRegionWidth(); }

    // Windows Scrolling
    inline float GetScrollX()                                          { return ImGui::GetScrollX(); }
    inline float GetScrollY()                                          { return ImGui::GetScrollY(); }
    inline float GetScrollMaxX()                                       { return ImGui::GetScrollMaxX(); }
    inline float GetScrollMaxY()                                       { return ImGui::GetScrollMaxY(); }
    inline void SetScrollX(float scrollX)                              { ImGui::SetScrollX(scrollX); }
    inline void SetScrollY(float scrollY)                              { ImGui::SetScrollY(scrollY); }
    inline void SetScrollHereX()                                       { ImGui::SetScrollHereX(); }
    inline void SetScrollHereX(float centerXRatio)                     { ImGui::SetScrollHereX(centerXRatio); }
    inline void SetScrollHereY()                                       { ImGui::SetScrollHereY(); }
    inline void SetScrollHereY(float centerYRatio)                     { ImGui::SetScrollHereY(centerYRatio); }
    inline void SetScrollFromPosX(float localX)                        { ImGui::SetScrollFromPosX(localX); }
    inline void SetScrollFromPosX(float localX, float centerXRatio)    { ImGui::SetScrollFromPosX(localX, centerXRatio); }
    inline void SetScrollFromPosY(float localY)                        { ImGui::SetScrollFromPosY(localY); }
    inline void SetScrollFromPosY(float localY, float centerYRatio)    { ImGui::SetScrollFromPosY(localY, centerYRatio); }

    // Parameters stacks (shared)
#ifdef SOL_IMGUI_ENABLE_FONT_MANIPULATORS
    inline void PushFont(ImFont* pFont)                                  { ImGui::PushFont(pFont); }
    inline void PopFont()                                                { ImGui::PopFont(); }
#endif // SOL_IMGUI_ENABLE_FONT_MANIPULATORS
    inline void PushStyleColor(int idx, int col)                                                 { ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), ImU32(col)); }
    inline void PushStyleColor(int idx, float colR, float colG, float colB, float colA)          { ImGui::PushStyleColor(static_cast<ImGuiCol>(idx), { colR, colG, colB, colA }); }
    inline void PopStyleColor()                                                                  { ImGui::PopStyleColor(); }
    inline void PopStyleColor(int count)                                                         { ImGui::PopStyleColor(count); }
    inline void PushStyleVar(int idx, float val)                                                 { ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(idx), val); }
    inline void PushStyleVar(int idx, float valX, float valY)                                    { ImGui::PushStyleVar(static_cast<ImGuiStyleVar>(idx), { valX, valY }); }
    inline void PopStyleVar()                                                                    { ImGui::PopStyleVar(); }
    inline void PopStyleVar(int count)                                                           { ImGui::PopStyleVar(count); }
    inline std::tuple<float, float, float, float> GetStyleColorVec4(int idx)                     { const auto col{ ImGui::GetStyleColorVec4(static_cast<ImGuiCol>(idx)) };  return std::make_tuple(col.x, col.y, col.z, col.w); }
#ifdef SOL_IMGUI_ENABLE_FONT_MANIPULATORS
    inline ImFont* GetFont()                                                                     { return ImGui::GetFont(); }
#endif // SOL_IMGUI_ENABLE_FONT_MANIPULATORS
    inline float GetFontSize()                                                                   { return ImGui::GetFontSize(); }
    inline std::tuple<float, float> GetFontTexUvWhitePixel()                                     { const auto vec2{ ImGui::GetFontTexUvWhitePixel() }; return std::make_tuple(vec2.x, vec2.y); }
    inline int GetColorU32(int idx, float alphaMul)                                              { return ImGui::GetColorU32(static_cast<ImGuiCol>(idx), alphaMul); }
    inline int GetColorU32(float colR, float colG, float colB, float colA)                       { return ImGui::GetColorU32({ colR, colG, colB, colA }); }
    inline int GetColorU32(int col)                                                              { return ImGui::GetColorU32(ImU32(col)); }

    // Parameters stacks (current window)
    inline void PushItemWidth(float itemWidth)                                 { ImGui::PushItemWidth(itemWidth); }
    inline void PopItemWidth()                                                 { ImGui::PopItemWidth(); }
    inline void SetNextItemWidth(float itemWidth)                              { ImGui::SetNextItemWidth(itemWidth); }
    inline float CalcItemWidth()                                               { return ImGui::CalcItemWidth(); }
    inline void PushTextWrapPos()                                              { ImGui::PushTextWrapPos(); }
    inline void PushTextWrapPos(float wrapLocalPosX)                           { ImGui::PushTextWrapPos(wrapLocalPosX); }
    inline void PopTextWrapPos()                                               { ImGui::PopTextWrapPos(); }
    inline void PushAllowKeyboardFocus(bool allowKeyboardFocus)                { ImGui::PushAllowKeyboardFocus(allowKeyboardFocus); }
    inline void PopAllowKeyboardFocus()                                        { ImGui::PopAllowKeyboardFocus(); }
    inline void PushButtonRepeat(bool repeat)                                  { ImGui::PushButtonRepeat(repeat); }
    inline void PopButtonRepeat()                                              { ImGui::PopButtonRepeat(); }

    // Cursor / Layout
    inline void Separator()                                                    { ImGui::Separator(); }
    inline void SameLine()                                                     { ImGui::SameLine(); }
    inline void SameLine(float offsetFromStartX)                               { ImGui::SameLine(offsetFromStartX); }
    inline void SameLine(float offsetFromStartX, float spacing)                { ImGui::SameLine(offsetFromStartX, spacing); }
    inline void NewLine()                                                      { ImGui::NewLine(); }
    inline void Spacing()                                                      { ImGui::Spacing(); }
    inline void Dummy(float sizeX, float sizeY)                                { ImGui::Dummy({ sizeX, sizeY }); }
    inline void Indent()                                                       { ImGui::Indent(); }
    inline void Indent(float indentW)                                          { ImGui::Indent(indentW); }
    inline void Unindent()                                                     { ImGui::Unindent(); }
    inline void Unindent(float indentW)                                        { ImGui::Unindent(indentW); }
    inline void BeginGroup()                                                   { ImGui::BeginGroup(); }
    inline void EndGroup()                                                     { ImGui::EndGroup(); }
    inline std::tuple<float, float> GetCursorPos()                             { const auto vec2{ ImGui::GetCursorPos() };  return std::make_tuple(vec2.x, vec2.y); }
    inline float GetCursorPosX()                                               { return ImGui::GetCursorPosX(); }
    inline float GetCursorPosY()                                               { return ImGui::GetCursorPosY(); }
    inline void SetCursorPos(float localX, float localY)                       { ImGui::SetCursorPos({ localX, localY }); }
    inline void SetCursorPosX(float localX)                                    { ImGui::SetCursorPosX(localX); }
    inline void SetCursorPosY(float localY)                                    { ImGui::SetCursorPosY(localY); }
    inline std::tuple<float, float> GetCursorStartPos()                        { const auto vec2{ ImGui::GetCursorStartPos() };  return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetCursorScreenPos()                       { const auto vec2{ ImGui::GetCursorScreenPos() };  return std::make_tuple(vec2.x, vec2.y); }
    inline void SetCursorScreenPos(float posX, float posY)                     { ImGui::SetCursorScreenPos({ posX, posY }); }
    inline void AlignTextToFramePadding()                                      { ImGui::AlignTextToFramePadding(); }
    inline float GetTextLineHeight()                                           { return ImGui::GetTextLineHeight(); }
    inline float GetTextLineHeightWithSpacing()                                { return ImGui::GetTextLineHeightWithSpacing(); }
    inline float GetFrameHeight()                                              { return ImGui::GetFrameHeight(); }
    inline float GetFrameHeightWithSpacing()                                   { return ImGui::GetFrameHeightWithSpacing(); }

    // ID stack / scopes
    inline void PushID(const std::string& stringID)                                             { ImGui::PushID(stringID.c_str()); }
    inline void PushID(int intID)                                                               { ImGui::PushID(intID); }
    inline void PopID()                                                                         { ImGui::PopID(); }
    inline int GetID(const std::string& stringID)                                               { return ImGui::GetID(stringID.c_str()); }

    // Widgets: Text
    inline void TextUnformatted(const std::string& text)                                               { ImGui::TextUnformatted(text.c_str()); }
    inline void Text(const std::string& text)                                                          { ImGui::Text(text.c_str()); }
    inline void TextColored(float colR, float colG, float colB, float colA, const std::string& text)   { ImGui::TextColored({ colR, colG, colB, colA }, text.c_str()); }
    inline void TextDisabled(const std::string& text)                                                  { ImGui::TextDisabled(text.c_str()); }
    inline void TextWrapped(const std::string text)                                                    { ImGui::TextWrapped(text.c_str()); }
    inline void LabelText(const std::string& label, const std::string& text)                           { ImGui::LabelText(label.c_str(), text.c_str()); }
    inline void BulletText(const std::string& text)                                                    { ImGui::BulletText(text.c_str()); }

    // Widgets: Main
    inline bool Button(const std::string& label)                                                { return ImGui::Button(label.c_str()); }
    inline bool Button(const std::string& label, float sizeX, float sizeY)                      { return ImGui::Button(label.c_str(), { sizeX, sizeY }); }
    inline bool SmallButton(const std::string& label)                                           { return ImGui::SmallButton(label.c_str()); }
    inline bool InvisibleButton(const std::string& stringID, float sizeX, float sizeY)          { return ImGui::InvisibleButton(stringID.c_str(), { sizeX, sizeY }); }
    inline bool ArrowButton(const std::string& stringID, int dir)                               { return ImGui::ArrowButton(stringID.c_str(), static_cast<ImGuiDir>(dir)); }
    inline void Image()                                                                         { /* TODO: Image(...) ==> UNSUPPORTED */ }
    inline void ImageButton()                                                                   { /* TODO: ImageButton(...) ==> UNSUPPORTED */ }
    inline std::tuple<bool, bool> Checkbox(const std::string& label, bool v)
    {
        bool value{ v };
        bool pressed = ImGui::Checkbox(label.c_str(), &value);

        return std::make_tuple(value, pressed);
    }
    inline bool CheckboxFlags()                                                                      { return false; /* TODO: CheckboxFlags(...) ==> UNSUPPORTED */ }
    inline bool RadioButton(const std::string& label, bool active)                                   { return ImGui::RadioButton(label.c_str(), active); }
    inline std::tuple<int, bool> RadioButton(const std::string& label, int v, int vButton)           { bool ret{ ImGui::RadioButton(label.c_str(), &v, vButton) }; return std::make_tuple(v, ret); }
    inline void ProgressBar(float fraction)                                                          { ImGui::ProgressBar(fraction); }
    inline void ProgressBar(float fraction, float sizeX, float sizeY)                                { ImGui::ProgressBar(fraction, { sizeX, sizeY }); }
    inline void ProgressBar(float fraction, float sizeX, float sizeY, const std::string& overlay)    { ImGui::ProgressBar(fraction, { sizeX, sizeY }, overlay.c_str()); }
    inline void Bullet()                                                                             { ImGui::Bullet(); }

    // Widgets: Combo Box
    inline bool BeginCombo(const std::string& label, const std::string& previewValue)                { return ImGui::BeginCombo(label.c_str(), previewValue.c_str()); }
    inline bool BeginCombo(const std::string& label, const std::string& previewValue, int flags)     { return ImGui::BeginCombo(label.c_str(), previewValue.c_str(), static_cast<ImGuiComboFlags>(flags)); }
    inline void EndCombo()                                                                           { ImGui::EndCombo(); }
    inline std::tuple<int, bool> Combo(const std::string& label, int currentItem, const sol::table& items, int itemsCount)
    {
        std::vector<std::string> strings;
        for (int i{ 1 }; i <= itemsCount; i++)
        {
            const auto& stringItem = items.get<sol::optional<std::string>>(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char*> cstrings;
        for (auto& string : strings)
            cstrings.push_back(string.c_str());

        bool clicked = ImGui::Combo(label.c_str(), &currentItem, cstrings.data(), itemsCount);
        return std::make_tuple(currentItem, clicked);
    }
    inline std::tuple<int, bool> Combo(const std::string& label, int currentItem, const sol::table& items, int itemsCount, int popupMaxHeightInItems)
    {
        std::vector<std::string> strings;
        for (int i{ 1 }; i <= itemsCount; i++)
        {
            const auto& stringItem = items.get<sol::optional<std::string>>(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char*> cstrings;
        for (auto& string : strings)
            cstrings.push_back(string.c_str());

        bool clicked = ImGui::Combo(label.c_str(), &currentItem, cstrings.data(), itemsCount, popupMaxHeightInItems);
        return std::make_tuple(currentItem, clicked);
    }
    inline std::tuple<int, bool> Combo(const std::string& label, int currentItem, const std::string& itemsSeparatedByZeros)
    {
        bool clicked = ImGui::Combo(label.c_str(), &currentItem, itemsSeparatedByZeros.c_str());
        return std::make_tuple(currentItem, clicked);
    }
    inline std::tuple<int, bool> Combo(const std::string& label, int currentItem, const std::string& itemsSeparatedByZeros, int popupMaxHeightInItems)
    {
        bool clicked = ImGui::Combo(label.c_str(), &currentItem, itemsSeparatedByZeros.c_str(), popupMaxHeightInItems);
        return std::make_tuple(currentItem, clicked);
    }
    // TODO: 3rd Combo from ImGui not Supported

    // Widgets: Drags
    inline std::tuple<float, bool> DragFloat(const std::string& label, float v)                                                                                     { bool used = ImGui::DragFloat(label.c_str(), &v); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed)                                                                      { bool used = ImGui::DragFloat(label.c_str(), &v, v_speed); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min)                                                         { bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max)                                            { bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max, const std::string& format)                 { bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max, format.c_str()); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> DragFloat(const std::string& label, float v, float v_speed, float v_min, float v_max, const std::string& format, int flags)      { bool used = ImGui::DragFloat(label.c_str(), &v, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v, used); }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::DragFloat2(label.c_str(), value);

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v, float v_speed)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::DragFloat2(label.c_str(), value, v_speed);

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v, float v_speed, float v_min)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min);

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max);

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat2(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::DragFloat2(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::DragFloat3(label.c_str(), value);

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v, float v_speed)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::DragFloat3(label.c_str(), value, v_speed);

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v, float v_speed, float v_min)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min);

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max);

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat3(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::DragFloat3(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::DragFloat4(label.c_str(), value);

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v, float v_speed)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::DragFloat4(label.c_str(), value, v_speed);

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v, float v_speed, float v_min)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min);

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max);

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> DragFloat4(const std::string& label, const sol::table& v, float v_speed, float v_min, float v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::DragFloat4(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline void DragFloatRange2()                                                                                                                     { /* TODO: DragFloatRange2(...) ==> UNSUPPORTED */ }
    inline std::tuple<int, bool> DragInt(const std::string& label, int v)                                                                             { bool used = ImGui::DragInt(label.c_str(), &v); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed)                                                              { bool used = ImGui::DragInt(label.c_str(), &v, v_speed); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min)                                                   { bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max)                                        { bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min, v_max); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max, const std::string& format)             { bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min, v_max, format.c_str()); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> DragInt(const std::string& label, int v, float v_speed, int v_min, int v_max, const std::string& format, int flags)  { bool used = ImGui::DragInt(label.c_str(), &v, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v, used); }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::DragInt2(label.c_str(), value);

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v, float v_speed)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::DragInt2(label.c_str(), value, v_speed);

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v, float v_speed, int v_min)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min);

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min, v_max);

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt2(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::DragInt2(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::DragInt3(label.c_str(), value);

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v, float v_speed)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::DragInt3(label.c_str(), value, v_speed);

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v, float v_speed, int v_min)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min);

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min, v_max);

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt3(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::DragInt3(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::DragInt4(label.c_str(), value);

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v, float v_speed)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::DragInt4(label.c_str(), value, v_speed);

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v, float v_speed, int v_min)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min);

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min, v_max);

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min, v_max, format.c_str());

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> DragInt4(const std::string& label, const sol::table& v, float v_speed, int v_min, int v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::DragInt4(label.c_str(), value, v_speed, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline void DragIntRange2()                                                                               { /* TODO: DragIntRange2(...) ==> UNSUPPORTED */ }
    inline void DragScalar()                                                                                  { /* TODO: DragScalar(...) ==> UNSUPPORTED */ }
    inline void DragScalarN()                                                                                 { /* TODO: DragScalarN(...) ==> UNSUPPORTED */ }

    // Widgets: Sliders
    inline std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max)                                              { bool used = ImGui::SliderFloat(label.c_str(), &v, v_min, v_max); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max, const std::string& format)                   { bool used = ImGui::SliderFloat(label.c_str(), &v, v_min, v_max, format.c_str()); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> SliderFloat(const std::string& label, float v, float v_min, float v_max, const std::string& format, int flags)        { bool used = ImGui::SliderFloat(label.c_str(), &v, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v, used); }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v, float v_min, float v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max);

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v, float v_min, float v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max, format.c_str());

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat2(const std::string& label, const sol::table& v, float v_min, float v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::SliderFloat2(label.c_str(), value, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v, float v_min, float v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max);

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[3]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v, float v_min, float v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max, format.c_str());

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[3]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat3(const std::string& label, const sol::table& v, float v_min, float v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::SliderFloat3(label.c_str(), value, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[3]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v, float v_min, float v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max);

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v, float v_min, float v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max, format.c_str());

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> SliderFloat4(const std::string& label, const sol::table& v, float v_min, float v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::SliderFloat4(label.c_str(), value, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad)                                                                                    { bool used = ImGui::SliderAngle(label.c_str(), &v_rad); return std::make_tuple(v_rad, used); }
    inline std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min)                                                               { bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min); return std::make_tuple(v_rad, used); }
    inline std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max)                                          { bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min, v_degrees_max); return std::make_tuple(v_rad, used); }
    inline std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max, const std::string& format)               { bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min, v_degrees_max, format.c_str()); return std::make_tuple(v_rad, used); }
    inline std::tuple<float, bool> SliderAngle(const std::string& label, float v_rad, float v_degrees_min, float v_degrees_max, const std::string& format, int flags)    { bool used = ImGui::SliderAngle(label.c_str(), &v_rad, v_degrees_min, v_degrees_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v_rad, used); }
    inline std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max)                                                                        { bool used = ImGui::SliderInt(label.c_str(), &v, v_min, v_max); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max, const std::string& format)                                             { bool used = ImGui::SliderInt(label.c_str(), &v, v_min, v_max, format.c_str()); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> SliderInt(const std::string& label, int v, int v_min, int v_max, const std::string& format, int flags)                                  { bool used = ImGui::SliderInt(label.c_str(), &v, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v, used); }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min, int v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::SliderInt2(label.c_str(), value, v_min, v_max);

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min, int v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::SliderInt2(label.c_str(), value, v_min, v_max, format.c_str());

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt2(const std::string& label, const sol::table& v, int v_min, int v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::SliderInt2(label.c_str(), value, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min, int v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::SliderInt3(label.c_str(), value, v_min, v_max);

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min, int v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::SliderInt3(label.c_str(), value, v_min, v_max, format.c_str());

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt3(const std::string& label, const sol::table& v, int v_min, int v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::SliderInt3(label.c_str(), value, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min, int v_max)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::SliderInt4(label.c_str(), value, v_min, v_max);

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min, int v_max, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::SliderInt4(label.c_str(), value, v_min, v_max, format.c_str());

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<int>>, bool> SliderInt4(const std::string& label, const sol::table& v, int v_min, int v_max, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::SliderInt4(label.c_str(), value, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags));

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline void SliderScalar()                                                                                                                                                    { /* TODO: SliderScalar(...) ==> UNSUPPORTED */ }
    inline void SliderScalarN()                                                                                                                                                   { /* TODO: SliderScalarN(...) ==> UNSUPPORTED */ }
    inline std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min, float v_max)                                            { bool used = ImGui::VSliderFloat(label.c_str(), { sizeX, sizeY }, &v, v_min, v_max); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min, float v_max, const std::string& format)                 { bool used = ImGui::VSliderFloat(label.c_str(), { sizeX, sizeY }, &v, v_min, v_max, format.c_str()); return std::make_tuple(v, used); }
    inline std::tuple<float, bool> VSliderFloat(const std::string& label, float sizeX, float sizeY, float v, float v_min, float v_max, const std::string& format, int flags)      { bool used = ImGui::VSliderFloat(label.c_str(), { sizeX, sizeY }, &v, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max)                                                      { bool used = ImGui::VSliderInt(label.c_str(), { sizeX, sizeY }, &v, v_min, v_max); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max, const std::string& format)                           { bool used = ImGui::VSliderInt(label.c_str(), { sizeX, sizeY }, &v, v_min, v_max, format.c_str()); return std::make_tuple(v, used); }
    inline std::tuple<int, bool> VSliderInt(const std::string& label, float sizeX, float sizeY, int v, int v_min, int v_max, const std::string& format, int flags)                { bool used = ImGui::VSliderInt(label.c_str(), { sizeX, sizeY }, &v, v_min, v_max, format.c_str(), static_cast<ImGuiSliderFlags>(flags)); return std::make_tuple(v, used); }
    inline void VSliderScalar()                                                                                                                                                   { /* TODO: VSliderScalar(...) ==> UNSUPPORTED */ }

    // Widgets: Input with Keyboard
    inline std::tuple<std::string, bool> InputText(const std::string& label, std::string text, unsigned int buf_size)                                                   { text.resize(buf_size); bool selected = ImGui::InputText(label.c_str(), &text[0], buf_size); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<std::string, bool> InputText(const std::string& label, std::string text, unsigned int buf_size, int flags)                                        { text.resize(buf_size); bool selected = ImGui::InputText(label.c_str(), &text[0], buf_size, static_cast<ImGuiInputTextFlags>(flags)); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size)                                          { text.resize(buf_size); bool selected = ImGui::InputTextMultiline(label.c_str(), &text[0], buf_size); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size, float sizeX, float sizeY)                { text.resize(buf_size); bool selected = ImGui::InputTextMultiline(label.c_str(), &text[0], buf_size, { sizeX, sizeY }); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<std::string, bool> InputTextMultiline(const std::string& label, std::string text, unsigned int buf_size, float sizeX, float sizeY, int flags)     { text.resize(buf_size); bool selected = ImGui::InputTextMultiline(label.c_str(), &text[0], buf_size, { sizeX, sizeY }, static_cast<ImGuiInputTextFlags>(flags)); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<std::string, bool> InputTextWithHint(const std::string& label, const std::string& hint, std::string text, unsigned int buf_size)                  { text.resize(buf_size); bool selected = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), &text[0], buf_size); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<std::string, bool> InputTextWithHint(const std::string& label, const std::string& hint, std::string text, unsigned int buf_size, int flags)       { text.resize(buf_size); bool selected = ImGui::InputTextWithHint(label.c_str(), hint.c_str(), &text[0], buf_size, static_cast<ImGuiInputTextFlags>(flags)); return std::make_tuple(text.c_str(), selected); }
    inline std::tuple<float, bool> InputFloat(const std::string& label, float v)                                                                                        { bool selected = ImGui::InputFloat(label.c_str(), &v); return std::make_tuple(v, selected); }
    inline std::tuple<float, bool> InputFloat(const std::string& label, float v, float step)                                                                            { bool selected = ImGui::InputFloat(label.c_str(), &v, step); return std::make_tuple(v, selected); }
    inline std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast)                                                           { bool selected = ImGui::InputFloat(label.c_str(), &v, step, step_fast); return std::make_tuple(v, selected); }
    inline std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast, const std::string& format)                                { bool selected = ImGui::InputFloat(label.c_str(), &v, step, step_fast, format.c_str()); return std::make_tuple(v, selected); }
    inline std::tuple<float, bool> InputFloat(const std::string& label, float v, float step, float step_fast, const std::string& format, int flags)                     { bool selected = ImGui::InputFloat(label.c_str(), &v, step, step_fast, format.c_str(), static_cast<ImGuiInputTextFlags>(flags)); return std::make_tuple(v, selected); }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::InputFloat2(label.c_str(), value);

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::InputFloat2(label.c_str(), value, format.c_str());

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat2(const std::string& label, const sol::table& v, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[2] = { float(v1), float(v2) };
        bool used = ImGui::InputFloat2(label.c_str(), value, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));

        sol::as_table_t float2 = sol::as_table(std::vector<float>{
            value[0], value[1]
        });

        return std::make_tuple(float2, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::InputFloat3(label.c_str(), value);

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::InputFloat3(label.c_str(), value, format.c_str());

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat3(const std::string& label, const sol::table& v, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[3] = { float(v1), float(v2), float(v3) };
        bool used = ImGui::InputFloat3(label.c_str(), value, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));

        sol::as_table_t float3 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(float3, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::InputFloat4(label.c_str(), value);

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v, const std::string& format)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::InputFloat4(label.c_str(), value, format.c_str());

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<float>>, bool> InputFloat4(const std::string& label, const sol::table& v, const std::string& format, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float value[4] = { float(v1), float(v2), float(v3), float(v4) };
        bool used = ImGui::InputFloat4(label.c_str(), value, format.c_str(), static_cast<ImGuiInputTextFlags>(flags));

        sol::as_table_t float4 = sol::as_table(std::vector<float>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(float4, used);
    }
    inline std::tuple<int, bool> InputInt(const std::string& label, int v)                                                        { bool selected = ImGui::InputInt(label.c_str(), &v); return std::make_tuple(v, selected); }
    inline std::tuple<int, bool> InputInt(const std::string& label, int v, int step)                                              { bool selected = ImGui::InputInt(label.c_str(), &v, step); return std::make_tuple(v, selected); }
    inline std::tuple<int, bool> InputInt(const std::string& label, int v, int step, int step_fast)                               { bool selected = ImGui::InputInt(label.c_str(), &v, step, step_fast); return std::make_tuple(v, selected); }
    inline std::tuple<int, bool> InputInt(const std::string& label, int v, int step, int step_fast, int flags)                    { bool selected = ImGui::InputInt(label.c_str(), &v, step, step_fast, static_cast<ImGuiInputTextFlags>(flags)); return std::make_tuple(v, selected); }
    inline std::tuple <sol::as_table_t<std::vector<int>>, bool> InputInt2(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::InputInt2(label.c_str(), value);

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<int>>, bool> InputInt2(const std::string& label, const sol::table& v, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[2] = { int(v1), int(v2) };
        bool used = ImGui::InputInt2(label.c_str(), value, static_cast<ImGuiInputTextFlags>(flags));

        sol::as_table_t int2 = sol::as_table(std::vector<int>{
            value[0], value[1]
        });

        return std::make_tuple(int2, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<int>>, bool> InputInt3(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::InputInt3(label.c_str(), value);

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<int>>, bool> InputInt3(const std::string& label, const sol::table& v, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[3] = { int(v1), int(v2), int(v3) };
        bool used = ImGui::InputInt3(label.c_str(), value, static_cast<ImGuiInputTextFlags>(flags));

        sol::as_table_t int3 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2]
        });

        return std::make_tuple(int3, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<int>>, bool> InputInt4(const std::string& label, const sol::table& v)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::InputInt4(label.c_str(), value);

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple <sol::as_table_t<std::vector<int>>, bool> InputInt4(const std::string& label, const sol::table& v, int flags)
    {
        const lua_Number  v1{ v[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v2{ v[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v3{ v[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          v4{ v[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        int value[4] = { int(v1), int(v2), int(v3), int(v4) };
        bool used = ImGui::InputInt4(label.c_str(), value, static_cast<ImGuiInputTextFlags>(flags));

        sol::as_table_t int4 = sol::as_table(std::vector<int>{
            value[0], value[1], value[2], value[3]
        });

        return std::make_tuple(int4, used);
    }
    inline std::tuple<double, bool> InputDouble(const std::string& label, double v)                                                                          { bool selected = ImGui::InputDouble(label.c_str(), &v); return std::make_tuple(v, selected); }
    inline std::tuple<double, bool> InputDouble(const std::string& label, double v, double step)                                                             { bool selected = ImGui::InputDouble(label.c_str(), &v, step); return std::make_tuple(v, selected); }
    inline std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast)                                           { bool selected = ImGui::InputDouble(label.c_str(), &v, step, step_fast); return std::make_tuple(v, selected); }
    inline std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast, const std::string& format)                { bool selected = ImGui::InputDouble(label.c_str(), &v, step, step_fast, format.c_str()); return std::make_tuple(v, selected); }
    inline std::tuple<double, bool> InputDouble(const std::string& label, double v, double step, double step_fast, const std::string& format, int flags)     { bool selected = ImGui::InputDouble(label.c_str(), &v, step, step_fast, format.c_str(), static_cast<ImGuiInputTextFlags>(flags)); return std::make_tuple(v, selected); }
    inline void InputScalar()                                                                                                                                { /* TODO: InputScalar(...) ==> UNSUPPORTED */ }
    inline void InputScalarN()                                                                                                                               { /* TODO: InputScalarN(...) ==> UNSUPPORTED */ }

    // Widgets: Color Editor / Picker
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
            g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
            b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorEdit3(label.c_str(), color);

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
            color[0], color[1], color[2]
        });

        return std::make_tuple(rgb, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit3(const std::string& label, const sol::table& col, int flags)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorEdit3(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
            color[0], color[1], color[2]
        });

        return std::make_tuple(rgb, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorEdit4(label.c_str(), color);

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
            color[0], color[1], color[2], color[3]
        });

        return std::make_tuple(rgba, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorEdit4(const std::string& label, const sol::table& col, int flags)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorEdit4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
            color[0], color[1], color[2], color[3]
        });

        return std::make_tuple(rgba, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorPicker3(label.c_str(), color);

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
            color[0], color[1], color[2]
        });

        return std::make_tuple(rgb, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker3(const std::string& label, const sol::table& col, int flags)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[3] = { float(r), float(g), float(b) };
        bool used = ImGui::ColorPicker3(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgb = sol::as_table(std::vector<float>{
            color[0], color[1], color[2]
        });

        return std::make_tuple(rgb, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorPicker4(label.c_str(), color);

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
            color[0], color[1], color[2], color[3]
        });

        return std::make_tuple(rgba, used);
    }
    inline std::tuple<sol::as_table_t<std::vector<float>>, bool> ColorPicker4(const std::string& label, const sol::table& col, int flags)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        float color[4] = { float(r), float(g), float(b), float(a) };
        bool used = ImGui::ColorPicker4(label.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));

        sol::as_table_t rgba = sol::as_table(std::vector<float>{
            color[0], color[1], color[2], color[3]
        });

        return std::make_tuple(rgba, used);
    }
    inline bool ColorButton(const std::string& desc_id, const sol::table& col)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        const ImVec4 color{ float(r), float(g), float(b), float(a) };
        return ImGui::ColorButton(desc_id.c_str(), color);
    }
    inline bool ColorButton(const std::string& desc_id, const sol::table& col, int flags)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        const ImVec4 color{ float(r), float(g), float(b), float(a) };
        return ImGui::ColorButton(desc_id.c_str(), color, static_cast<ImGuiColorEditFlags>(flags));
    }
    inline bool ColorButton(const std::string& desc_id, const sol::table& col, int flags, float sizeX, float sizeY)
    {
        const lua_Number  r{ col[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ col[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ col[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ col[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };
        const ImVec4 color{ float(r), float(g), float(b), float(a) };
        return ImGui::ColorButton(desc_id.c_str(), color, static_cast<ImGuiColorEditFlags>(flags), { sizeX, sizeY });
    }
    inline void SetColorEditOptions(int flags)                                                                     { ImGui::SetColorEditOptions(static_cast<ImGuiColorEditFlags>(flags)); }

    // Widgets: Trees
    inline bool TreeNode(const std::string& label)                                                                 { return ImGui::TreeNode(label.c_str()); }
    inline bool TreeNode(const std::string& label, const std::string& fmt)                                         { return ImGui::TreeNode(label.c_str(), fmt.c_str()); }
    /* TODO: TreeNodeV(...) (2) ==> UNSUPPORTED */
    inline bool TreeNodeEx(const std::string& label)                                                               { return ImGui::TreeNodeEx(label.c_str()); }
    inline bool TreeNodeEx(const std::string& label, int flags)                                                    { return ImGui::TreeNodeEx(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags)); }
    inline bool TreeNodeEx(const std::string& label, int flags, const std::string& fmt)                            { return ImGui::TreeNodeEx(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags), fmt.c_str()); }
    /* TODO: TreeNodeExV(...) (2) ==> UNSUPPORTED */
    inline void TreePush(const std::string& str_id)                                                                { ImGui::TreePush(str_id.c_str()); }
    /* TODO: TreePush(const void*) ==> UNSUPPORTED */
    inline void TreePop()                                                                                          { ImGui::TreePop(); }
    inline float GetTreeNodeToLabelSpacing()                                                                       { return ImGui::GetTreeNodeToLabelSpacing(); }
    inline bool CollapsingHeader(const std::string& label)                                                         { return ImGui::CollapsingHeader(label.c_str()); }
    inline bool CollapsingHeader(const std::string& label, int flags)                                              { return ImGui::CollapsingHeader(label.c_str(), static_cast<ImGuiTreeNodeFlags>(flags)); }
    inline std::tuple<bool, bool> CollapsingHeader(const std::string& label, bool open)                            { bool notCollapsed = ImGui::CollapsingHeader(label.c_str(), &open); return std::make_tuple(open, notCollapsed); }
    inline std::tuple<bool, bool> CollapsingHeader(const std::string& label, bool open, int flags)                 { bool notCollapsed = ImGui::CollapsingHeader(label.c_str(), &open, static_cast<ImGuiTreeNodeFlags>(flags)); return std::make_tuple(open, notCollapsed); }
    inline void SetNextItemOpen(bool is_open)                                                                      { ImGui::SetNextItemOpen(is_open); }
    inline void SetNextItemOpen(bool is_open, int cond)                                                            { ImGui::SetNextItemOpen(is_open, static_cast<ImGuiCond>(cond)); }

    // Widgets: Selectables
    // TODO: Only one of Selectable variations is possible due to same parameters for Lua
    inline bool Selectable(const std::string& label)                                                               { return ImGui::Selectable(label.c_str()); }
    inline bool Selectable(const std::string& label, bool selected)                                                { ImGui::Selectable(label.c_str(), &selected); return selected; }
    inline bool Selectable(const std::string& label, bool selected, int flags)                                     { ImGui::Selectable(label.c_str(), &selected, static_cast<ImGuiSelectableFlags>(flags)); return selected; }
    inline bool Selectable(const std::string& label, bool selected, int flags, float sizeX, float sizeY)           { ImGui::Selectable(label.c_str(), &selected, static_cast<ImGuiSelectableFlags>(flags), { sizeX, sizeY }); return selected; }

    // Widgets: List Boxes
    inline std::tuple<int, bool> ListBox(const std::string& label, int current_item, const sol::table& items, int items_count)
    {
        std::vector<std::string> strings;
        for (int i{ 1 }; i <= items_count; i++)
        {
            const auto& stringItem = items.get<sol::optional<std::string>>(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char*> cstrings;
        for (auto& string : strings)
            cstrings.push_back(string.c_str());

        bool clicked = ImGui::ListBox(label.c_str(), &current_item, cstrings.data(), items_count);
        return std::make_tuple(current_item, clicked);
    }
    inline std::tuple<int, bool> ListBox(const std::string& label, int current_item, const sol::table& items, int items_count, int height_in_items)
    {
        std::vector<std::string> strings;
        for (int i{ 1 }; i <= items_count; i++)
        {
            const auto& stringItem = items.get<sol::optional<std::string>>(i);
            strings.push_back(stringItem.value_or("Missing"));
        }

        std::vector<const char*> cstrings;
        for (auto& string : strings)
            cstrings.push_back(string.c_str());

        bool clicked = ImGui::ListBox(label.c_str(), &current_item, cstrings.data(), items_count, height_in_items);
        return std::make_tuple(current_item, clicked);
    }
    inline bool BeginListBox(const std::string& label)                                               { return ImGui::BeginListBox(label.c_str()); }
    inline bool BeginListBox(const std::string& label, float sizeX, float sizeY)                     { return ImGui::BeginListBox(label.c_str(), { sizeX, sizeY }); }
    inline void EndListBox()                                                                         { ImGui::EndListBox(); }

    // Widgets: Data Plotting
    /* TODO: Widgets Data Plotting ==> UNSUPPORTED (barely used and quite long functions) */

    // Widgets: Value() helpers
    inline void Value(const std::string& prefix, bool b)                                            { ImGui::Value(prefix.c_str(), b); }
    inline void Value(const std::string& prefix, int v)                                             { ImGui::Value(prefix.c_str(), v); }
    inline void Value(const std::string& prefix, unsigned int v)                                    { ImGui::Value(prefix.c_str(), v); }
    inline void Value(const std::string& prefix, float v)                                           { ImGui::Value(prefix.c_str(), v); }
    inline void Value(const std::string& prefix, float v, const std::string& float_format)          { ImGui::Value(prefix.c_str(), v, float_format.c_str()); }

    // Widgets: Menus
    inline bool BeginMenuBar()                                                                                                    { return ImGui::BeginMenuBar(); }
    inline void EndMenuBar()                                                                                                      { ImGui::EndMenuBar(); }
    inline bool BeginMainMenuBar()                                                                                                { return ImGui::BeginMainMenuBar(); }
    inline void EndMainMenuBar()                                                                                                  { ImGui::EndMainMenuBar(); }
    inline bool BeginMenu(const std::string& label)                                                                               { return ImGui::BeginMenu(label.c_str()); }
    inline bool BeginMenu(const std::string& label, bool enabled)                                                                 { return ImGui::BeginMenu(label.c_str(), enabled); }
    inline void EndMenu()                                                                                                         { ImGui::EndMenu(); }
    inline bool MenuItem(const std::string& label)                                                                                { return ImGui::MenuItem(label.c_str()); }
    inline bool MenuItem(const std::string& label, const std::string& shortcut)                                                   { return ImGui::MenuItem(label.c_str(), shortcut.c_str()); }
    inline std::tuple<bool, bool> MenuItem(const std::string& label, const std::string& shortcut, bool selected)                  { bool activated = ImGui::MenuItem(label.c_str(), shortcut.c_str(), &selected); return std::make_tuple(selected, activated); }
    inline std::tuple<bool, bool> MenuItem(const std::string& label, const std::string& shortcut, bool selected, bool enabled)    { bool activated = ImGui::MenuItem(label.c_str(), shortcut.c_str(), &selected, enabled); return std::make_tuple(selected, activated); }

    // Tooltips
    inline void BeginTooltip()                                                 { ImGui::BeginTooltip(); }
    inline void EndTooltip()                                                   { ImGui::EndTooltip(); }
    inline void SetTooltip(const std::string& fmt)                             { ImGui::SetTooltip(fmt.c_str()); }
    inline void SetTooltipV()                                                  { /* TODO: SetTooltipV(...) ==> UNSUPPORTED */ }

    // Popups, Modals
    inline bool BeginPopup(const std::string& str_id)                                   { return ImGui::BeginPopup(str_id.c_str()); }
    inline bool BeginPopup(const std::string& str_id, int flags)                        { return ImGui::BeginPopup(str_id.c_str(), static_cast<ImGuiWindowFlags>(flags)); }
    inline bool BeginPopupModal(const std::string& name)                                { return ImGui::BeginPopupModal(name.c_str()); }
    inline bool BeginPopupModal(const std::string& name, int flags)                     { return ImGui::BeginPopupModal(name.c_str(), NULL, static_cast<ImGuiWindowFlags>(flags)); }
    inline bool BeginPopupModal(const std::string& name, bool open)                     { return ImGui::BeginPopupModal(name.c_str(), &open); }
    inline bool BeginPopupModal(const std::string& name, bool open, int flags)          { return ImGui::BeginPopupModal(name.c_str(), &open, static_cast<ImGuiWindowFlags>(flags)); }
    inline void EndPopup()                                                              { ImGui::EndPopup(); }
    inline void OpenPopup(const std::string& str_id)                                    { ImGui::OpenPopup(str_id.c_str()); }
    inline void OpenPopup(const std::string& str_id, int popup_flags)                   { ImGui::OpenPopup(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags)); }
    inline void CloseCurrentPopup()                                                     { ImGui::CloseCurrentPopup(); }
    inline bool BeginPopupContextItem()                                                 { return ImGui::BeginPopupContextItem(); }
    inline bool BeginPopupContextItem(const std::string& str_id)                        { return ImGui::BeginPopupContextItem(str_id.c_str()); }
    inline bool BeginPopupContextItem(const std::string& str_id, int popup_flags)       { return ImGui::BeginPopupContextItem(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags)); }
    inline bool BeginPopupContextWindow()                                               { return ImGui::BeginPopupContextWindow(); }
    inline bool BeginPopupContextWindow(const std::string& str_id)                      { return ImGui::BeginPopupContextWindow(str_id.c_str()); }
    inline bool BeginPopupContextWindow(const std::string& str_id, int popup_flags)     { return ImGui::BeginPopupContextWindow(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags)); }
    inline bool BeginPopupContextVoid()                                                 { return ImGui::BeginPopupContextVoid(); }
    inline bool BeginPopupContextVoid(const std::string& str_id)                        { return ImGui::BeginPopupContextVoid(str_id.c_str()); }
    inline bool BeginPopupContextVoid(const std::string& str_id, int popup_flags)       { return ImGui::BeginPopupContextVoid(str_id.c_str(), static_cast<ImGuiPopupFlags>(popup_flags)); }
    inline bool IsPopupOpen(const std::string& str_id)                                  { return ImGui::IsPopupOpen(str_id.c_str()); }
    inline bool IsPopupOpen(const std::string& str_id, int popup_flags)                 { return ImGui::IsPopupOpen(str_id.c_str(), popup_flags); }

    //Tables
    inline bool BeginTable(const std::string& str_id, int columns)                                                                          { return ImGui::BeginTable(str_id.c_str(), columns); }
    inline bool BeginTable(const std::string& str_id, int columns, int flags)                                                               { return ImGui::BeginTable(str_id.c_str(), columns, static_cast<ImGuiTableFlags>(flags)); }
    inline bool BeginTable(const std::string& str_id, int columns, int flags, float outer_sizeX, float outer_sizeY)                         { return ImGui::BeginTable(str_id.c_str(), columns, static_cast<ImGuiTableFlags>(flags), { outer_sizeX, outer_sizeY }); }
    inline bool BeginTable(const std::string& str_id, int columns, int flags, float outer_sizeX, float outer_sizeY, float inner_width)      { return ImGui::BeginTable(str_id.c_str(), columns, static_cast<ImGuiTableFlags>(flags), { outer_sizeX, outer_sizeY }, inner_width); }
    inline void EndTable()                                                                                                                  { ImGui::EndTable(); }
    inline void TableNextRow()                                                                                                              { ImGui::TableNextRow(); }
    inline void TableNextRow(int flags)                                                                                                     { ImGui::TableNextRow(static_cast<ImGuiTableRowFlags>(flags)); }
    inline void TableNextRow(int flags, float min_row_height)                                                                               { ImGui::TableNextRow(static_cast<ImGuiTableRowFlags>(flags), min_row_height); }
    inline bool TableNextColumn()                                                                                                           { return ImGui::TableNextColumn(); }
    inline bool TableSetColumnIndex(int column_n)                                                                                           { return ImGui::TableSetColumnIndex(column_n); }
    inline void TableSetupColumn(const std::string& label)                                                                                  { ImGui::TableSetupColumn(label.c_str()); }
    inline void TableSetupColumn(const std::string& label, int flags)                                                                       { ImGui::TableSetupColumn(label.c_str(), static_cast<ImGuiTableColumnFlags>(flags)); }
    inline void TableSetupColumn(const std::string& label, int flags, float init_width_or_weight)                                           { ImGui::TableSetupColumn(label.c_str(), static_cast<ImGuiTableColumnFlags>(flags), init_width_or_weight); }
    inline void TableSetupColumn(const std::string& label, int flags, float init_width_or_weight, int user_id)                              { ImGui::TableSetupColumn(label.c_str(), static_cast<ImGuiTableColumnFlags>(flags), init_width_or_weight, ImU32(user_id)); }
    inline void TableSetupScrollFreeze(int cols, int rows)                                                                                  { ImGui::TableSetupScrollFreeze(cols, rows); }
    inline void TableHeadersRow()                                                                                                           { ImGui::TableHeadersRow(); }
    inline void TableHeader(const std::string& label)                                                                                       { ImGui::TableHeader(label.c_str()); }
    inline ImGuiTableSortSpecs* TableGetSortSpecs()                                                                                         { return ImGui::TableGetSortSpecs(); }
    inline int TableGetColumnCount()                                                                                                        { return ImGui::TableGetColumnCount(); }
    inline int TableGetColumnIndex()                                                                                                        { return ImGui::TableGetColumnIndex(); }
    inline int TableGetRowIndex()                                                                                                           { return ImGui::TableGetRowIndex(); }
    inline std::string TableGetColumnName()                                                                                                 { return std::string(ImGui::TableGetColumnName()); }
    inline std::string TableGetColumnName(int column_n)                                                                                     { return std::string(ImGui::TableGetColumnName(column_n)); }
    inline ImGuiTableColumnFlags TableGetColumnFlags()                                                                                      { return ImGui::TableGetColumnFlags(); }
    inline ImGuiTableColumnFlags TableGetColumnFlags(int column_n)                                                                          { return ImGui::TableGetColumnFlags(column_n); }
    inline void TableSetBgColor(int target, int color)                                                                                      { ImGui::TableSetBgColor(static_cast<ImGuiTableBgTarget>(target), ImU32(color)); }
    inline void TableSetBgColor(int target, float colR, float colG, float colB, float colA)                                                 { ImGui::TableSetBgColor(static_cast<ImGuiTableBgTarget>(target), ImGui::ColorConvertFloat4ToU32({ colR, colG, colB, colA })); }
    inline void TableSetBgColor(int target, int color, int column_n)                                                                        { ImGui::TableSetBgColor(static_cast<ImGuiTableBgTarget>(target), ImU32(color), column_n); }
    inline void TableSetBgColor(int target, float colR, float colG, float colB, float colA, int column_n)                                   { ImGui::TableSetBgColor(static_cast<ImGuiTableBgTarget>(target), ImGui::ColorConvertFloat4ToU32({ colR, colG, colB, colA }), column_n); }

    // Columns
    inline void Columns()                                                        { ImGui::Columns(); }
    inline void Columns(int count)                                               { ImGui::Columns(count); }
    inline void Columns(int count, const std::string& id)                        { ImGui::Columns(count, id.c_str()); }
    inline void Columns(int count, const std::string& id, bool border)           { ImGui::Columns(count, id.c_str(), border); }
    inline void NextColumn()                                                     { ImGui::NextColumn(); }
    inline int GetColumnIndex()                                                  { return ImGui::GetColumnIndex(); }
    inline float GetColumnWidth()                                                { return ImGui::GetColumnWidth(); }
    inline float GetColumnWidth(int column_index)                                { return ImGui::GetColumnWidth(column_index); }
    inline void SetColumnWidth(int column_index, float width)                    { ImGui::SetColumnWidth(column_index, width); }
    inline float GetColumnOffset()                                               { return ImGui::GetColumnOffset(); }
    inline float GetColumnOffset(int column_index)                               { return ImGui::GetColumnOffset(column_index); }
    inline void SetColumnOffset(int column_index, float offset_x)                { ImGui::SetColumnOffset(column_index, offset_x); }
    inline int GetColumnsCount()                                                 { return ImGui::GetColumnsCount(); }

    // Tab Bars, Tabs
    inline bool BeginTabBar(const std::string& str_id)                                              { return ImGui::BeginTabBar(str_id.c_str()); }
    inline bool BeginTabBar(const std::string& str_id, int flags)                                   { return ImGui::BeginTabBar(str_id.c_str(), static_cast<ImGuiTabBarFlags>(flags)); }
    inline void EndTabBar()                                                                         { ImGui::EndTabBar(); }
    inline bool BeginTabItem(const std::string& label)                                              { return ImGui::BeginTabItem(label.c_str()); }
    inline bool BeginTabItem(const std::string& label, int flags)                                   { return ImGui::BeginTabItem(label.c_str(), NULL, static_cast<ImGuiTabItemFlags>(flags)); }
    inline std::tuple<bool, bool> BeginTabItem(const std::string& label, bool open)                 { bool selected = ImGui::BeginTabItem(label.c_str(), &open); return std::make_tuple(open, selected); }
    inline std::tuple<bool, bool> BeginTabItem(const std::string& label, bool open, int flags)      { bool selected = ImGui::BeginTabItem(label.c_str(), &open, static_cast<ImGuiTabItemFlags>(flags)); return std::make_tuple(open, selected); }
    inline void EndTabItem()                                                                        { ImGui::EndTabItem(); }
    inline void SetTabItemClosed(const std::string& tab_or_docked_window_label)                     { ImGui::SetTabItemClosed(tab_or_docked_window_label.c_str()); }

    // Drag and Drop
    // TODO: Drag and Drop ==> UNSUPPORTED

    // Clipping
    inline void PushClipRect(float min_x, float min_y, float max_x, float max_y, bool intersect_current)   { ImGui::PushClipRect({ min_x, min_y }, { max_x, max_y }, intersect_current); }
    inline void PopClipRect()                                                                              { ImGui::PopClipRect(); }

    // Focus, Activation
    inline void SetItemDefaultFocus()                                          { ImGui::SetItemDefaultFocus(); }
    inline void SetKeyboardFocusHere()                                         { ImGui::SetKeyboardFocusHere(); }
    inline void SetKeyboardFocusHere(int offset)                               { ImGui::SetKeyboardFocusHere(offset); }

    // Item/Widgets Utilities
    inline bool IsItemHovered()                                                { return ImGui::IsItemHovered(); }
    inline bool IsItemHovered(int flags)                                       { return ImGui::IsItemHovered(static_cast<ImGuiHoveredFlags>(flags)); }
    inline bool IsItemActive()                                                 { return ImGui::IsItemActive(); }
    inline bool IsItemFocused()                                                { return ImGui::IsItemFocused(); }
    inline bool IsItemClicked()                                                { return ImGui::IsItemClicked(); }
    inline bool IsItemClicked(int mouse_button)                                { return ImGui::IsItemClicked(static_cast<ImGuiMouseButton>(mouse_button)); }
    inline bool IsItemVisible()                                                { return ImGui::IsItemVisible(); }
    inline bool IsItemEdited()                                                 { return ImGui::IsItemEdited(); }
    inline bool IsItemActivated()                                              { return ImGui::IsItemActivated(); }
    inline bool IsItemDeactivated()                                            { return ImGui::IsItemDeactivated(); }
    inline bool IsItemDeactivatedAfterEdit()                                   { return ImGui::IsItemDeactivatedAfterEdit(); }
    inline bool IsItemToggledOpen()                                            { return ImGui::IsItemToggledOpen(); }
    inline bool IsAnyItemHovered()                                             { return ImGui::IsAnyItemHovered(); }
    inline bool IsAnyItemActive()                                              { return ImGui::IsAnyItemActive(); }
    inline bool IsAnyItemFocused()                                             { return ImGui::IsAnyItemFocused(); }
    inline std::tuple<float, float> GetItemRectMin()                           { const auto vec2{ ImGui::GetItemRectMin() }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetItemRectMax()                           { const auto vec2{ ImGui::GetItemRectMax() }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetItemRectSize()                          { const auto vec2{ ImGui::GetItemRectSize() }; return std::make_tuple(vec2.x, vec2.y); }
    inline void SetItemAllowOverlap()                                          { ImGui::SetItemAllowOverlap(); }

    // Miscellaneous Utilities
    inline bool IsRectVisible(float sizeX, float sizeY)                                    { return ImGui::IsRectVisible({ sizeX, sizeY }); }
    inline bool IsRectVisible(float minX, float minY, float maxX, float maxY)              { return ImGui::IsRectVisible({ minX, minY }, { maxX, maxY }); }
    inline double GetTime()                                                                { return ImGui::GetTime(); }
    inline int GetFrameCount()                                                             { return ImGui::GetFrameCount(); }
    inline ImDrawList* GetBackgroundDrawList()                                             { return ImGui::GetBackgroundDrawList(); }
    inline ImDrawList* GetForegroundDrawList()                                             { return ImGui::GetForegroundDrawList(); }
    /* TODO: GetDrawListSharedData() ==> UNSUPPORTED */
    inline std::string GetStyleColorName(int idx)                                          { return std::string(ImGui::GetStyleColorName(static_cast<ImGuiCol>(idx))); }
    /* TODO: SetStateStorage(), GetStateStorage(), CalcListClipping() ==> UNSUPPORTED */
    inline bool BeginChildFrame(unsigned int id, float sizeX, float sizeY)                 { return ImGui::BeginChildFrame(id, { sizeX, sizeY }); }
    inline bool BeginChildFrame(unsigned int id, float sizeX, float sizeY, int flags)      { return ImGui::BeginChildFrame(id, { sizeX, sizeY }, static_cast<ImGuiWindowFlags>(flags)); }
    inline void EndChildFrame()                                                            { return ImGui::EndChildFrame(); }

    // Text Utilities
    inline std::tuple<float, float> CalcTextSize(const std::string& text)                                                      { const auto vec2{ ImGui::CalcTextSize(text.c_str()) }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> CalcTextSize(const std::string& text, bool hide_text_after_double_hash)                    { const auto vec2{ ImGui::CalcTextSize(text.c_str(), nullptr, hide_text_after_double_hash) }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> CalcTextSize(const std::string& text, bool hide_text_after_double_hash, float wrap_width)  { const auto vec2{ ImGui::CalcTextSize(text.c_str(), nullptr, hide_text_after_double_hash, wrap_width) }; return std::make_tuple(vec2.x, vec2.y); }

    // Color Utilities
    inline sol::as_table_t<std::vector<float>> ColorConvertU32ToFloat4(unsigned int in)
    {
        const auto vec4 = ImGui::ColorConvertU32ToFloat4(in);
        sol::as_table_t rgba = sol::as_table(std::vector<float>{
            vec4.x, vec4.y, vec4.z, vec4.w
        });

        return rgba;
    }
    inline unsigned int ColorConvertFloat4ToU32(const sol::table& rgba)
    {
        const lua_Number  r{ rgba[1].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          g{ rgba[2].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          b{ rgba[3].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) },
                          a{ rgba[4].get<std::optional<lua_Number>>().value_or(static_cast<lua_Number>(0)) };

        return ImGui::ColorConvertFloat4ToU32({ float(r), float(g), float(b), float(a) });
    }
    inline std::tuple<float, float, float> ColorConvertRGBtoHSV(float r, float g, float b)
    {
        float h{}, s{}, v{};
        ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);
        return std::make_tuple(h, s, v);
    }
    inline std::tuple<float, float, float> ColorConvertHSVtoRGB(float h, float s, float v)
    {
        float r{}, g{}, b{};
        ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
        return std::make_tuple(r, g, b);
    }

    // Inputs Utilities: Mouse
    inline bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y)               { return ImGui::IsMouseHoveringRect({ min_x, min_y }, { max_x, max_y }); }
    inline bool IsMouseHoveringRect(float min_x, float min_y, float max_x, float max_y, bool clip)    { return ImGui::IsMouseHoveringRect({ min_x, min_y }, { max_x, max_y }, clip); }
    inline std::tuple<float, float> GetMousePos()                                                     { const auto vec2{ ImGui::GetMousePos() }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetMousePosOnOpeningCurrentPopup()                                { const auto vec2{ ImGui::GetMousePosOnOpeningCurrentPopup() }; return std::make_tuple(vec2.x, vec2.y); }
    inline bool IsMouseDragging(int button)                                                           { return ImGui::IsMouseDragging(static_cast<ImGuiMouseButton>(button)); }
    inline bool IsMouseDragging(int button, float lock_threshold)                                     { return ImGui::IsMouseDragging(static_cast<ImGuiMouseButton>(button), lock_threshold); }
    inline std::tuple<float, float> GetMouseDragDelta()                                               { const auto vec2{ ImGui::GetMouseDragDelta() }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetMouseDragDelta(int button)                                     { const auto vec2{ ImGui::GetMouseDragDelta(static_cast<ImGuiMouseButton>(button)) }; return std::make_tuple(vec2.x, vec2.y); }
    inline std::tuple<float, float> GetMouseDragDelta(int button, float lock_threshold)               { const auto vec2{ ImGui::GetMouseDragDelta(static_cast<ImGuiMouseButton>(button), lock_threshold) }; return std::make_tuple(vec2.x, vec2.y); }
    inline void ResetMouseDragDelta()                                                                 { ImGui::ResetMouseDragDelta(); }
    inline void ResetMouseDragDelta(int button)                                                       { ImGui::ResetMouseDragDelta(static_cast<ImGuiMouseButton>(button)); }

    // Clipboard Utilities
    inline std::string GetClipboardText()                                                             { return std::string(ImGui::GetClipboardText()); }
    inline void SetClipboardText(const std::string& text)                                             { ImGui::SetClipboardText(text.c_str()); }

    // Drawing APIs
    // Primitives
    inline void ImDrawListAddLine(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, int col)                                                                                                    { drawlist->AddLine({ p1X, p1Y }, { p2X, p2Y }, ImU32(col)); }
    inline void ImDrawListAddLine(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, int col, float thickness)                                                                                   { drawlist->AddLine({ p1X, p1Y }, { p2X, p2Y }, ImU32(col), thickness); }
    inline void ImDrawListAddRect(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col)                                                                                        { drawlist->AddRect({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col)); }
    inline void ImDrawListAddRect(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col, float rounding)                                                                        { drawlist->AddRect({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col), rounding); }
    inline void ImDrawListAddRect(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col, float rounding, int rounding_corners)                                                  { drawlist->AddRect({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col), rounding, static_cast<ImDrawCornerFlags>(rounding_corners)); }
    inline void ImDrawListAddRect(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col, float rounding, int rounding_corners, float thickness)                                 { drawlist->AddRect({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col), rounding, static_cast<ImDrawCornerFlags>(rounding_corners), thickness); }
    inline void ImDrawListAddRectFilled(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col)                                                                                  { drawlist->AddRectFilled({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col)); }
    inline void ImDrawListAddRectFilled(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col, float rounding)                                                                  { drawlist->AddRectFilled({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col), rounding); }
    inline void ImDrawListAddRectFilled(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col, float rounding, int rounding_corners)                                            { drawlist->AddRectFilled({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col), rounding, static_cast<ImDrawCornerFlags>(rounding_corners)); }
    inline void ImDrawListAddRectFilledMultiColor(ImDrawList* drawlist, float p_minX, float p_minY, float p_maxX, float p_maxY, int col_upr_left, int col_upr_right, int col_bot_right, int col_bot_left)       { drawlist->AddRectFilledMultiColor({ p_minX, p_minY }, { p_maxX, p_maxY }, ImU32(col_upr_left), ImU32(col_upr_right), ImU32(col_bot_right), ImU32(col_bot_left)); }
    inline void ImDrawListAddQuad(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float p4X, float p4Y, int col)                                                        { drawlist->AddQuad({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, { p4X, p4Y }, ImU32(col)); }
    inline void ImDrawListAddQuad(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float p4X, float p4Y, int col, float thickness)                                       { drawlist->AddQuad({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, { p4X, p4Y }, ImU32(col), thickness); }
    inline void ImDrawListAddQuadFilled(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float p4X, float p4Y, int col)                                                  { drawlist->AddQuadFilled({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, { p4X, p4Y }, ImU32(col)); }
    inline void ImDrawListAddTriangle(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, int col)                                                                          { drawlist->AddTriangle({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, ImU32(col)); }
    inline void ImDrawListAddTriangle(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, int col, float thickness)                                                         { drawlist->AddTriangle({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, ImU32(col), thickness); }
    inline void ImDrawListAddTriangleFilled(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, int col)                                                                    { drawlist->AddTriangleFilled({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, ImU32(col)); }
    inline void ImDrawListAddCircle(ImDrawList* drawlist, float centerX, float centerY, float radius, int col)                                                                                                  { drawlist->AddCircle({ centerX, centerY }, radius, ImU32(col)); }
    inline void ImDrawListAddCircle(ImDrawList* drawlist, float centerX, float centerY, float radius, int col, int num_segments)                                                                                { drawlist->AddCircle({ centerX, centerY }, radius, ImU32(col), num_segments); }
    inline void ImDrawListAddCircle(ImDrawList* drawlist, float centerX, float centerY, float radius, int col, int num_segments, float thickness)                                                               { drawlist->AddCircle({ centerX, centerY }, radius, ImU32(col), num_segments, thickness); }
    inline void ImDrawListAddCircleFilled(ImDrawList* drawlist, float centerX, float centerY, float radius, int col)                                                                                            { drawlist->AddCircleFilled({ centerX, centerY }, radius, ImU32(col)); }
    inline void ImDrawListAddCircleFilled(ImDrawList* drawlist, float centerX, float centerY, float radius, int col, int num_segments)                                                                          { drawlist->AddCircleFilled({ centerX, centerY }, radius, ImU32(col), num_segments); }
    inline void ImDrawListAddNgon(ImDrawList* drawlist, float centerX, float centerY, float radius, int col, int num_segments)                                                                                  { drawlist->AddNgon({ centerX, centerY }, radius, ImU32(col), num_segments); }
    inline void ImDrawListAddNgon(ImDrawList* drawlist, float centerX, float centerY, float radius, int col, int num_segments, float thickness)                                                                 { drawlist->AddNgon({ centerX, centerY }, radius, ImU32(col), num_segments, thickness); }
    inline void ImDrawListAddNgonFilled(ImDrawList* drawlist, float centerX, float centerY, float radius, int col, int num_segments)                                                                            { drawlist->AddNgonFilled({ centerX, centerY }, radius, ImU32(col), num_segments); }
    inline void ImDrawListAddText(ImDrawList* drawlist, float posX, float posY, int col, const std::string& text_begin)                                                                                         { drawlist->AddText({ posX, posY }, ImU32(col), text_begin.c_str()); }
    inline void ImDrawListAddText(ImDrawList* drawlist, float font_size, float posX, float posY, int col, const std::string& text_begin)                                                                        { drawlist->AddText(ImGui::GetFont(), font_size, { posX, posY }, ImU32(col), text_begin.c_str()); }
    inline void ImDrawListAddText(ImDrawList* drawlist, float font_size, float posX, float posY, int col, const std::string& text_begin, float wrap_width)                                                      { drawlist->AddText(ImGui::GetFont(), font_size, { posX, posY }, ImU32(col), text_begin.c_str(), NULL, wrap_width); }
    // TODO
    // inline void ImDrawListAddText(ImDrawList* drawlist, float font_size, float posX, float posY, int col, const std::string& text_begin, float wrap_width, sol::table float cpu_fine_clip_rect)                 { drawlist->AddText(ImGui::GetFont(), font_size, { posX, posY }, ImU32(col), text_begin.c_str(), NULL, wrap_width, cpu_fine_clip_rect); }
    // inline void ImDrawListAddPolyline(ImDrawList* drawlist, sol::table points, int num_points, int col, bool closed, float thickness)                                                                           { drawlist->AddPolyline(points, num_points, ImU32(col), &closed, thickness); }
    // inline void ImDrawListAddConvexPolyFilled(ImDrawList* drawlist, sol::table points, int num_points, int col)                                                                                                 { drawlist->AddConvexPolyFilled(points, num_points, ImU32(col)); }
    inline void ImDrawListAddBezierCubic(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float p4X, float p4Y, int col, float thickness)                                { drawlist->AddBezierCubic({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, { p4X, p4Y }, ImU32(col), thickness); }
    inline void ImDrawListAddBezierCubic(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, float p4X, float p4Y, int col, float thickness, int num_segments)              { drawlist->AddBezierCubic({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, { p4X, p4Y }, ImU32(col), thickness, num_segments); }
    inline void ImDrawListAddBezierQuadratic(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, int col, float thickness)                                                  { drawlist->AddBezierQuadratic({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, ImU32(col), thickness); }
    inline void ImDrawListAddBezierQuadratic(ImDrawList* drawlist, float p1X, float p1Y, float p2X, float p2Y, float p3X, float p3Y, int col, float thickness, int num_segments)                                { drawlist->AddBezierQuadratic({ p1X, p1Y }, { p2X, p2Y }, { p3X, p3Y }, ImU32(col), thickness, num_segments); }


    inline void InitEnums(sol::state& lua)
    {
#pragma region Window Flags
        lua.new_enum("ImGuiWindowFlags",
            "None"                           , ImGuiWindowFlags_None,
            "NoTitleBar"                     , ImGuiWindowFlags_NoTitleBar,
            "NoResize"                       , ImGuiWindowFlags_NoResize,
            "NoMove"                         , ImGuiWindowFlags_NoMove,
            "NoScrollbar"                    , ImGuiWindowFlags_NoScrollbar,
            "NoScrollWithMouse"              , ImGuiWindowFlags_NoScrollWithMouse,
            "NoCollapse"                     , ImGuiWindowFlags_NoCollapse,
            "AlwaysAutoResize"               , ImGuiWindowFlags_AlwaysAutoResize,
            "NoBackground"                   , ImGuiWindowFlags_NoBackground,
            "NoSavedSettings"                , ImGuiWindowFlags_NoSavedSettings,
            "NoMouseInputs"                  , ImGuiWindowFlags_NoMouseInputs,
            "MenuBar"                        , ImGuiWindowFlags_MenuBar,
            "HorizontalScrollbar"            , ImGuiWindowFlags_HorizontalScrollbar,
            "NoFocusOnAppearing"             , ImGuiWindowFlags_NoFocusOnAppearing,
            "NoBringToFrontOnFocus"          , ImGuiWindowFlags_NoBringToFrontOnFocus,
            "AlwaysVerticalScrollbar"        , ImGuiWindowFlags_AlwaysVerticalScrollbar,
            "AlwaysHorizontalScrollbar"      , ImGuiWindowFlags_AlwaysHorizontalScrollbar,
            "AlwaysUseWindowPadding"         , ImGuiWindowFlags_AlwaysUseWindowPadding,
            "NoNavInputs"                    , ImGuiWindowFlags_NoNavInputs,
            "NoNavFocus"                     , ImGuiWindowFlags_NoNavFocus,
            "UnsavedDocument"                , ImGuiWindowFlags_UnsavedDocument,
            "NoNav"                          , ImGuiWindowFlags_NoNav,
            "NoDecoration"                   , ImGuiWindowFlags_NoDecoration,
            "NoInputs"                       , ImGuiWindowFlags_NoInputs,
            // [Internal]
            "NavFlattened"                   , ImGuiWindowFlags_NavFlattened,
            "ChildWindow"                    , ImGuiWindowFlags_ChildWindow,
            "Tooltip"                        , ImGuiWindowFlags_Tooltip,
            "Popup"                          , ImGuiWindowFlags_Popup,
            "Modal"                          , ImGuiWindowFlags_Modal,
            "ChildMenu"                      , ImGuiWindowFlags_ChildMenu
        );
#pragma endregion Window Flags

#pragma region Focused Flags
        lua.new_enum("ImGuiFocusedFlags",
            "None"                           , ImGuiFocusedFlags_None,
            "ChildWindows"                   , ImGuiFocusedFlags_ChildWindows,
            "RootWindow"                     , ImGuiFocusedFlags_RootWindow,
            "AnyWindow"                      , ImGuiFocusedFlags_AnyWindow,
            "RootAndChildWindows"            , ImGuiFocusedFlags_RootAndChildWindows
        );
#pragma endregion Focused Flags

#pragma region Hovered Flags
        lua.new_enum("ImGuiHoveredFlags",
            "None"                           , ImGuiHoveredFlags_None,
            "ChildWindows"                   , ImGuiHoveredFlags_ChildWindows,
            "RootWindow"                     , ImGuiHoveredFlags_RootWindow,
            "AnyWindow"                      , ImGuiHoveredFlags_AnyWindow,
            "AllowWhenBlockedByPopup"        , ImGuiHoveredFlags_AllowWhenBlockedByPopup,
            "AllowWhenBlockedByActiveItem"   , ImGuiHoveredFlags_AllowWhenBlockedByActiveItem,
            "AllowWhenOverlapped"            , ImGuiHoveredFlags_AllowWhenOverlapped,
            "AllowWhenDisabled"              , ImGuiHoveredFlags_AllowWhenDisabled,
            "RectOnly"                       , ImGuiHoveredFlags_RectOnly,
            "RootAndChildWindows"            , ImGuiHoveredFlags_RootAndChildWindows
        );
#pragma endregion Hovered Flags

#pragma region Cond
        lua.new_enum("ImGuiCond",
            "None"                           , ImGuiCond_None,
            "Always"                         , ImGuiCond_Always,
            "Once"                           , ImGuiCond_Once,
            "FirstUseEver"                   , ImGuiCond_FirstUseEver,
            "Appearing"                      , ImGuiCond_Appearing
        );
#pragma endregion Cond

#pragma region Col
        lua.new_enum("ImGuiCol",
            "Text"                           , ImGuiCol_Text,
            "TextDisabled"                   , ImGuiCol_TextDisabled,
            "WindowBg"                       , ImGuiCol_WindowBg,
            "ChildBg"                        , ImGuiCol_ChildBg,
            "PopupBg"                        , ImGuiCol_PopupBg,
            "Border"                         , ImGuiCol_Border,
            "BorderShadow"                   , ImGuiCol_BorderShadow,
            "FrameBg"                        , ImGuiCol_FrameBg,
            "FrameBgHovered"                 , ImGuiCol_FrameBgHovered,
            "FrameBgActive"                  , ImGuiCol_FrameBgActive,
            "TitleBg"                        , ImGuiCol_TitleBg,
            "TitleBgActive"                  , ImGuiCol_TitleBgActive,
            "TitleBgCollapsed"               , ImGuiCol_TitleBgCollapsed,
            "MenuBarBg"                      , ImGuiCol_MenuBarBg,
            "ScrollbarBg"                    , ImGuiCol_ScrollbarBg,
            "ScrollbarGrab"                  , ImGuiCol_ScrollbarGrab,
            "ScrollbarGrabHovered"           , ImGuiCol_ScrollbarGrabHovered,
            "ScrollbarGrabActive"            , ImGuiCol_ScrollbarGrabActive,
            "CheckMark"                      , ImGuiCol_CheckMark,
            "SliderGrab"                     , ImGuiCol_SliderGrab,
            "SliderGrabActive"               , ImGuiCol_SliderGrabActive,
            "Button"                         , ImGuiCol_Button,
            "ButtonHovered"                  , ImGuiCol_ButtonHovered,
            "ButtonActive"                   , ImGuiCol_ButtonActive,
            "Header"                         , ImGuiCol_Header,
            "HeaderHovered"                  , ImGuiCol_HeaderHovered,
            "HeaderActive"                   , ImGuiCol_HeaderActive,
            "Separator"                      , ImGuiCol_Separator,
            "SeparatorHovered"               , ImGuiCol_SeparatorHovered,
            "SeparatorActive"                , ImGuiCol_SeparatorActive,
            "ResizeGrip"                     , ImGuiCol_ResizeGrip,
            "ResizeGripHovered"              , ImGuiCol_ResizeGripHovered,
            "ResizeGripActive"               , ImGuiCol_ResizeGripActive,
            "Tab"                            , ImGuiCol_Tab,
            "TabHovered"                     , ImGuiCol_TabHovered,
            "TabActive"                      , ImGuiCol_TabActive,
            "TabUnfocused"                   , ImGuiCol_TabUnfocused,
            "TabUnfocusedActive"             , ImGuiCol_TabUnfocusedActive,
            "PlotLines"                      , ImGuiCol_PlotLines,
            "PlotLinesHovered"               , ImGuiCol_PlotLinesHovered,
            "PlotHistogram"                  , ImGuiCol_PlotHistogram,
            "PlotHistogramHovered"           , ImGuiCol_PlotHistogramHovered,
            "TableHeaderBg"                  , ImGuiCol_TableHeaderBg,
            "TableBorderStrong"              , ImGuiCol_TableBorderStrong,
            "TableBorderLight"               , ImGuiCol_TableBorderLight,
            "TableRowBg"                     , ImGuiCol_TableRowBg,
            "TableRowBgAlt"                  , ImGuiCol_TableRowBgAlt,
            "TextSelectedBg"                 , ImGuiCol_TextSelectedBg,
            "DragDropTarget"                 , ImGuiCol_DragDropTarget,
            "NavHighlight"                   , ImGuiCol_NavHighlight,
            "NavWindowingHighlight"          , ImGuiCol_NavWindowingHighlight,
            "NavWindowingDimBg"              , ImGuiCol_NavWindowingDimBg,
            "ModalWindowDimBg"               , ImGuiCol_ModalWindowDimBg,
            "COUNT"                          , ImGuiCol_COUNT
        );
#pragma endregion Col

#pragma region Style
        lua.new_enum("ImGuiStyleVar",
            "Alpha"                          , ImGuiStyleVar_Alpha,
            "WindowPadding"                  , ImGuiStyleVar_WindowPadding,
            "WindowRounding"                 , ImGuiStyleVar_WindowRounding,
            "WindowBorderSize"               , ImGuiStyleVar_WindowBorderSize,
            "WindowMinSize"                  , ImGuiStyleVar_WindowMinSize,
            "WindowTitleAlign"               , ImGuiStyleVar_WindowTitleAlign,
            "ChildRounding"                  , ImGuiStyleVar_ChildRounding,
            "ChildBorderSize"                , ImGuiStyleVar_ChildBorderSize,
            "PopupRounding"                  , ImGuiStyleVar_PopupRounding,
            "PopupBorderSize"                , ImGuiStyleVar_PopupBorderSize,
            "FramePadding"                   , ImGuiStyleVar_FramePadding,
            "FrameRounding"                  , ImGuiStyleVar_FrameRounding,
            "FrameBorderSize"                , ImGuiStyleVar_FrameBorderSize,
            "ItemSpacing"                    , ImGuiStyleVar_ItemSpacing,
            "ItemInnerSpacing"               , ImGuiStyleVar_ItemInnerSpacing,
            "IndentSpacing"                  , ImGuiStyleVar_IndentSpacing,
            "CellPadding"                    , ImGuiStyleVar_CellPadding,
            "ScrollbarSize"                  , ImGuiStyleVar_ScrollbarSize,
            "ScrollbarRounding"              , ImGuiStyleVar_ScrollbarRounding,
            "GrabMinSize"                    , ImGuiStyleVar_GrabMinSize,
            "GrabRounding"                   , ImGuiStyleVar_GrabRounding,
            "TabRounding"                    , ImGuiStyleVar_TabRounding,
            "SelectableTextAlign"            , ImGuiStyleVar_SelectableTextAlign,
            "ButtonTextAlign"                , ImGuiStyleVar_ButtonTextAlign,
            "COUNT"                          , ImGuiStyleVar_COUNT
        );
#pragma endregion Style

#pragma region Dir
        lua.new_enum("ImGuiDir",
            "None"                           , ImGuiDir_None,
            "Left"                           , ImGuiDir_Left,
            "Right"                          , ImGuiDir_Right,
            "Up"                             , ImGuiDir_Up,
            "Down"                           , ImGuiDir_Down,
            "COUNT"                          , ImGuiDir_COUNT
        );
#pragma endregion Dir

#pragma region Combo Flags
        lua.new_enum("ImGuiComboFlags",
            "None"                           , ImGuiComboFlags_None,
            "PopupAlignLeft"                 , ImGuiComboFlags_PopupAlignLeft,
            "HeightSmall"                    , ImGuiComboFlags_HeightSmall,
            "HeightRegular"                  , ImGuiComboFlags_HeightRegular,
            "HeightLarge"                    , ImGuiComboFlags_HeightLarge,
            "HeightLargest"                  , ImGuiComboFlags_HeightLargest,
            "NoArrowButton"                  , ImGuiComboFlags_NoArrowButton,
            "NoPreview"                      , ImGuiComboFlags_NoPreview,
            "HeightMask"                     , ImGuiComboFlags_HeightMask_
        );
#pragma endregion Combo Flags

#pragma region InputText Flags
        lua.new_enum("ImGuiInputTextFlags",
            "None"                           , ImGuiInputTextFlags_None,
            "CharsDecimal"                   , ImGuiInputTextFlags_CharsDecimal,
            "CharsHexadecimal"               , ImGuiInputTextFlags_CharsHexadecimal,
            "CharsUppercase"                 , ImGuiInputTextFlags_CharsUppercase,
            "CharsNoBlank"                   , ImGuiInputTextFlags_CharsNoBlank,
            "AutoSelectAll"                  , ImGuiInputTextFlags_AutoSelectAll,
            "EnterReturnsTrue"               , ImGuiInputTextFlags_EnterReturnsTrue,
            "CallbackCompletion"             , ImGuiInputTextFlags_CallbackCompletion,
            "CallbackHistory"                , ImGuiInputTextFlags_CallbackHistory,
            "CallbackAlways"                 , ImGuiInputTextFlags_CallbackAlways,
            "CallbackCharFilter"             , ImGuiInputTextFlags_CallbackCharFilter,
            "AllowTabInput"                  , ImGuiInputTextFlags_AllowTabInput,
            "CtrlEnterForNewLine"            , ImGuiInputTextFlags_CtrlEnterForNewLine,
            "NoHorizontalScroll"             , ImGuiInputTextFlags_NoHorizontalScroll,
            "AlwaysInsertMode"               , ImGuiInputTextFlags_AlwaysInsertMode,
            "ReadOnly"                       , ImGuiInputTextFlags_ReadOnly,
            "Password"                       , ImGuiInputTextFlags_Password,
            "NoUndoRedo"                     , ImGuiInputTextFlags_NoUndoRedo,
            "CharsScientific"                , ImGuiInputTextFlags_CharsScientific,
            "CallbackResize"                 , ImGuiInputTextFlags_CallbackResize,
            "CallbackEdit"                   , ImGuiInputTextFlags_CallbackEdit,
            // [Internal]
            "Multiline"                      , ImGuiInputTextFlags_Multiline,
            "NoMarkEdited"                   , ImGuiInputTextFlags_NoMarkEdited
        );
#pragma endregion InputText Flags

#pragma region Slider Flags
        lua.new_enum("ImGuiSliderFlags",
            "None"                           , ImGuiSliderFlags_None,
            "ClampOnInput"                   , ImGuiSliderFlags_ClampOnInput,
            "Logarithmic"                    , ImGuiSliderFlags_Logarithmic,
            "NoRoundToFormat"                , ImGuiSliderFlags_NoRoundToFormat,
            "NoInput"                        , ImGuiSliderFlags_NoInput
        );
#pragma endregion Slider Flags

#pragma region ColorEdit Flags
        lua.new_enum("ImGuiColorEditFlags",
            "None"                           , ImGuiColorEditFlags_None,
            "NoAlpha"                        , ImGuiColorEditFlags_NoAlpha,
            "NoPicker"                       , ImGuiColorEditFlags_NoPicker,
            "NoOptions"                      , ImGuiColorEditFlags_NoOptions,
            "NoSmallPreview"                 , ImGuiColorEditFlags_NoSmallPreview,
            "NoInputs"                       , ImGuiColorEditFlags_NoInputs,
            "NoTooltip"                      , ImGuiColorEditFlags_NoTooltip,
            "NoLabel"                        , ImGuiColorEditFlags_NoLabel,
            "NoSidePreview"                  , ImGuiColorEditFlags_NoSidePreview,
            "NoDragDrop"                     , ImGuiColorEditFlags_NoDragDrop,
            "NoBorder"                       , ImGuiColorEditFlags_NoBorder,

            "AlphaBar"                       , ImGuiColorEditFlags_AlphaBar,
            "AlphaPreview"                   , ImGuiColorEditFlags_AlphaPreview,
            "AlphaPreviewHalf"               , ImGuiColorEditFlags_AlphaPreviewHalf,
            "HDR"                            , ImGuiColorEditFlags_HDR,
            "DisplayRGB"                     , ImGuiColorEditFlags_DisplayRGB,
            "DisplayHSV"                     , ImGuiColorEditFlags_DisplayHSV,
            "DisplayHex"                     , ImGuiColorEditFlags_DisplayHex,
            "Uint8"                          , ImGuiColorEditFlags_Uint8,
            "Float"                          , ImGuiColorEditFlags_Float,
            "PickerHueBar"                   , ImGuiColorEditFlags_PickerHueBar,
            "PickerHueWheel"                 , ImGuiColorEditFlags_PickerHueWheel,
            "InputRGB"                       , ImGuiColorEditFlags_InputRGB,
            "InputHSV"                       , ImGuiColorEditFlags_InputHSV,

            "_OptionsDefault"                , ImGuiColorEditFlags__OptionsDefault,

            "_DisplayMask"                   , ImGuiColorEditFlags__DisplayMask,
            "_DataTypeMask"                  , ImGuiColorEditFlags__DataTypeMask,
            "_PickerMask"                    , ImGuiColorEditFlags__PickerMask,
            "_InputMask"                     , ImGuiColorEditFlags__InputMask
        );
#pragma endregion ColorEdit Flags

#pragma region TreeNode Flags
        lua.new_enum("ImGuiTreeNodeFlags",
            "None"                           , ImGuiTreeNodeFlags_None,
            "Selected"                       , ImGuiTreeNodeFlags_Selected,
            "Framed"                         , ImGuiTreeNodeFlags_Framed,
            "AllowItemOverlap"               , ImGuiTreeNodeFlags_AllowItemOverlap,
            "NoTreePushOnOpen"               , ImGuiTreeNodeFlags_NoTreePushOnOpen,
            "NoAutoOpenOnLog"                , ImGuiTreeNodeFlags_NoAutoOpenOnLog,
            "DefaultOpen"                    , ImGuiTreeNodeFlags_DefaultOpen,
            "OpenOnDoubleClick"              , ImGuiTreeNodeFlags_OpenOnDoubleClick,
            "OpenOnArrow"                    , ImGuiTreeNodeFlags_OpenOnArrow,
            "Leaf"                           , ImGuiTreeNodeFlags_Leaf,
            "Bullet"                         , ImGuiTreeNodeFlags_Bullet,
            "FramePadding"                   , ImGuiTreeNodeFlags_FramePadding,
            "SpanAvailWidth"                 , ImGuiTreeNodeFlags_SpanAvailWidth,
            "SpanFullWidth"                  , ImGuiTreeNodeFlags_SpanFullWidth,
            "NavLeftJumpsBackHere"           , ImGuiTreeNodeFlags_NavLeftJumpsBackHere,
            "CollapsingHeader"               , ImGuiTreeNodeFlags_CollapsingHeader
        );
#pragma endregion TreeNode Flags

#pragma region Selectable Flags
        lua.new_enum("ImGuiSelectableFlags",
            "None"                           , ImGuiSelectableFlags_None,
            "DontClosePopups"                , ImGuiSelectableFlags_DontClosePopups,
            "SpanAllColumns"                 , ImGuiSelectableFlags_SpanAllColumns,
            "AllowDoubleClick"               , ImGuiSelectableFlags_AllowDoubleClick,
            "Disabled"                       , ImGuiSelectableFlags_Disabled,
            "AllowItemOverlap"               , ImGuiSelectableFlags_AllowItemOverlap
        );
#pragma endregion Selectable Flags

#pragma region Popup Flags
        lua.new_enum("ImGuiPopupFlags",
            "None"                           , ImGuiPopupFlags_None,
            "MouseButtonLeft"                , ImGuiPopupFlags_MouseButtonLeft,
            "MouseButtonRight"               , ImGuiPopupFlags_MouseButtonRight,
            "MouseButtonMiddle"              , ImGuiPopupFlags_MouseButtonMiddle,
            "MouseButtonMask_"               , ImGuiPopupFlags_MouseButtonMask_,
            "MouseButtonDefault_"            , ImGuiPopupFlags_MouseButtonDefault_,
            "NoOpenOverExistingPopup"        , ImGuiPopupFlags_NoOpenOverExistingPopup,
            "NoOpenOverItems"                , ImGuiPopupFlags_NoOpenOverItems,
            "AnyPopupId"                     , ImGuiPopupFlags_AnyPopupId,
            "AnyPopupLevel"                  , ImGuiPopupFlags_AnyPopupLevel,
            "AnyPopup"                       , ImGuiPopupFlags_AnyPopup
        );
#pragma endregion Popup Flags

#pragma region Table Flags
        lua.new_enum("ImGuiTableFlags",
            // Features
            "None"                           , ImGuiTableFlags_None,
            "Resizable"                      , ImGuiTableFlags_Resizable,
            "Reorderable"                    , ImGuiTableFlags_Reorderable,
            "Hideable"                       , ImGuiTableFlags_Hideable,
            "Sortable"                       , ImGuiTableFlags_Sortable,
            "NoSavedSettings"                , ImGuiTableFlags_NoSavedSettings,
            "ContextMenuInBody"              , ImGuiTableFlags_ContextMenuInBody,
            // Decorations
            "RowBg"                          , ImGuiTableFlags_RowBg,
            "BordersInnerH"                  , ImGuiTableFlags_BordersInnerH,
            "BordersOuterH"                  , ImGuiTableFlags_BordersOuterH,
            "BordersInnerV"                  , ImGuiTableFlags_BordersInnerV,
            "BordersOuterV"                  , ImGuiTableFlags_BordersOuterV,
            "BordersH"                       , ImGuiTableFlags_BordersH,
            "BordersV"                       , ImGuiTableFlags_BordersV,
            "BordersInner"                   , ImGuiTableFlags_BordersInner,
            "BordersOuter"                   , ImGuiTableFlags_BordersOuter,
            "Borders"                        , ImGuiTableFlags_Borders,
            "NoBordersInBody"                , ImGuiTableFlags_NoBordersInBody,
            "NoBordersInBodyUntilResize"     , ImGuiTableFlags_NoBordersInBodyUntilResize,
            // Sizing Policy (read above for defaults)
            "SizingFixedFit"                 , ImGuiTableFlags_SizingFixedFit,
            "SizingFixedSame"                , ImGuiTableFlags_SizingFixedSame,
            "SizingStretchProp"              , ImGuiTableFlags_SizingStretchProp,
            "SizingStretchSame"              , ImGuiTableFlags_SizingStretchSame,
            // Sizing Extra Options
            "NoHostExtendX"                  , ImGuiTableFlags_NoHostExtendX,
            "NoHostExtendY"                  , ImGuiTableFlags_NoHostExtendY,
            "NoKeepColumnsVisible"           , ImGuiTableFlags_NoKeepColumnsVisible,
            "PreciseWidths"                  , ImGuiTableFlags_PreciseWidths,
            // Clipping
            "NoClip"                         , ImGuiTableFlags_NoClip,
            // Padding
            "PadOuterX"                      , ImGuiTableFlags_PadOuterX,
            "NoPadOuterX"                    , ImGuiTableFlags_NoPadOuterX,
            "NoPadInnerX"                    , ImGuiTableFlags_NoPadInnerX,
            // Scrolling
            "ScrollX"                        , ImGuiTableFlags_ScrollX,
            "ScrollY"                        , ImGuiTableFlags_ScrollY,
            // Sorting
            "SortMulti"                      , ImGuiTableFlags_SortMulti,
            "SortTristate"                   , ImGuiTableFlags_SortTristate,
            // [Internal] Combinations and masks
            "SizingMask"                     , ImGuiTableFlags_SizingMask_
        );
#pragma endregion Table Flags

#pragma region TableColumn Flags
        lua.new_enum("ImGuiTableColumnFlags",
        // Input configuration flags
            "None"                           , ImGuiTableColumnFlags_None,
            "DefaultHide"                    , ImGuiTableColumnFlags_DefaultHide,
            "DefaultSort"                    , ImGuiTableColumnFlags_DefaultSort,
            "WidthStretch"                   , ImGuiTableColumnFlags_WidthStretch,
            "WidthFixed"                     , ImGuiTableColumnFlags_WidthFixed,
            "NoResize"                       , ImGuiTableColumnFlags_NoResize,
            "NoReorder"                      , ImGuiTableColumnFlags_NoReorder,
            "NoHide"                         , ImGuiTableColumnFlags_NoHide,
            "NoClip"                         , ImGuiTableColumnFlags_NoClip,
            "NoSort"                         , ImGuiTableColumnFlags_NoSort,
            "NoSortAscending"                , ImGuiTableColumnFlags_NoSortAscending,
            "NoSortDescending"               , ImGuiTableColumnFlags_NoSortDescending,
            "NoHeaderWidth"                  , ImGuiTableColumnFlags_NoHeaderWidth,
            "PreferSortAscending"            , ImGuiTableColumnFlags_PreferSortAscending,
            "PreferSortDescending"           , ImGuiTableColumnFlags_PreferSortDescending,
            "IndentEnable"                   , ImGuiTableColumnFlags_IndentEnable,
            "IndentDisable"                  , ImGuiTableColumnFlags_IndentDisable,
            // Output status flags, read-only via TableGetColumnFlags()
            "IsEnabled"                      , ImGuiTableColumnFlags_IsEnabled,
            "IsVisible"                      , ImGuiTableColumnFlags_IsVisible,
            "IsSorted"                       , ImGuiTableColumnFlags_IsSorted,
            "IsHovered"                      , ImGuiTableColumnFlags_IsHovered,
            // [Internal] Combinations and masks
            "WidthMask_"                     , ImGuiTableColumnFlags_WidthMask_,
            "IndentMask_"                    , ImGuiTableColumnFlags_IndentMask_,
            "StatusMask_"                    , ImGuiTableColumnFlags_StatusMask_,
            "NoDirectResize_"                , ImGuiTableColumnFlags_NoDirectResize_
        );
#pragma endregion TableColumn Flags

#pragma region TableRow Flags
        lua.new_enum("ImGuiTableRowFlags",
            "None"                           , ImGuiTableRowFlags_None,
            "Headers"                        , ImGuiTableRowFlags_Headers
        );
#pragma endregion TableRow Flags

#pragma region TableBg Target
        lua.new_enum("ImGuiTableBgTarget",
            "None"                           , ImGuiTableBgTarget_None,
            "RowBg0"                         , ImGuiTableBgTarget_RowBg0,
            "RowBg1"                         , ImGuiTableBgTarget_RowBg1,
            "CellBg"                         , ImGuiTableBgTarget_CellBg
        );
#pragma endregion TableBg Target

#pragma region TabBar Flags
        lua.new_enum("ImGuiTabBarFlags",
            "None"                           , ImGuiTabBarFlags_None,
            "Reorderable"                    , ImGuiTabBarFlags_Reorderable,
            "AutoSelectNewTabs"              , ImGuiTabBarFlags_AutoSelectNewTabs,
            "TabListPopupButton"             , ImGuiTabBarFlags_TabListPopupButton,
            "NoCloseWithMiddleMouseButton"   , ImGuiTabBarFlags_NoCloseWithMiddleMouseButton,
            "NoTabListScrollingButtons"      , ImGuiTabBarFlags_NoTabListScrollingButtons,
            "NoTooltip"                      , ImGuiTabBarFlags_NoTooltip,
            "FittingPolicyResizeDown"        , ImGuiTabBarFlags_FittingPolicyResizeDown,
            "FittingPolicyScroll"            , ImGuiTabBarFlags_FittingPolicyScroll,
            "FittingPolicyMask_"             , ImGuiTabBarFlags_FittingPolicyMask_,
            "FittingPolicyDefault_"          , ImGuiTabBarFlags_FittingPolicyDefault_
        );
#pragma endregion TabBar Flags

#pragma region TabItem Flags
        lua.new_enum("ImGuiTabItemFlags",
            "None"                           , ImGuiTabItemFlags_None,
            "UnsavedDocument"                , ImGuiTabItemFlags_UnsavedDocument,
            "SetSelected"                    , ImGuiTabItemFlags_SetSelected,
            "NoCloseWithMiddleMouseButton"   , ImGuiTabItemFlags_NoCloseWithMiddleMouseButton,
            "NoPushId"                       , ImGuiTabItemFlags_NoPushId,
            "NoTooltip"                      , ImGuiTabItemFlags_NoTooltip,
            "NoReorder"                      , ImGuiTabItemFlags_NoReorder,
            "Leading"                        , ImGuiTabItemFlags_Leading,
            "Trailing"                       , ImGuiTabItemFlags_Trailing
        );
#pragma endregion TabItem Flags

#pragma region MouseButton
        lua.new_enum("ImGuiMouseButton",
            "Left"                           , ImGuiMouseButton_Left,
            "Right"                          , ImGuiMouseButton_Right,
            "Middle"                         , ImGuiMouseButton_Middle,
            "COUNT"                          , ImGuiMouseButton_COUNT
        );
#pragma endregion MouseButton

#pragma region ImDrawCorner Flags
        lua.new_enum("ImDrawCornerFlags",
            "None"                           , ImDrawCornerFlags_None,
            "TopLeft"                        , ImDrawCornerFlags_TopLeft,
            "TopRight"                       , ImDrawCornerFlags_TopRight,
            "BotLeft"                        , ImDrawCornerFlags_BotLeft,
            "BotRight"                       , ImDrawCornerFlags_BotRight,
            "Top"                            , ImDrawCornerFlags_Top,
            "Bot"                            , ImDrawCornerFlags_Bot,
            "Left"                           , ImDrawCornerFlags_Left,
            "Right"                          , ImDrawCornerFlags_Right,
            "All"                            , ImDrawCornerFlags_All
        );
#pragma endregion ImDrawCorner Flags
    }

    inline void InitBindings(sol::state& lua)
    {
        InitEnums(lua);

        sol::table ImGui = lua.create_named_table("ImGui");

#pragma region Windows
        ImGui.set_function("Begin"              , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(Begin),
                                                                sol::resolve<bool(const std::string&, int)>(Begin),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(Begin),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int)>(Begin)
                                                            ));
        ImGui.set_function("End"              , End);
#pragma endregion Windows

#pragma region Child Windows
        ImGui.set_function("BeginChild"            , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginChild),
                                                                sol::resolve<bool(const std::string&, float)>(BeginChild),
                                                                sol::resolve<bool(const std::string&, float, float)>(BeginChild),
                                                                sol::resolve<bool(const std::string&, float, float, bool)>(BeginChild),
                                                                sol::resolve<bool(const std::string&, float, float, bool, int)>(BeginChild)
                                                            ));
        ImGui.set_function("EndChild"            , EndChild);
#pragma endregion Child Windows

#pragma region Window Utilities
        ImGui.set_function("IsWindowAppearing"      , IsWindowAppearing);
        ImGui.set_function("IsWindowCollapsed"      , IsWindowCollapsed);
        ImGui.set_function("IsWindowFocused"        , sol::overload(
                                                                sol::resolve<bool()>(IsWindowFocused),
                                                                sol::resolve<bool(int)>(IsWindowFocused)
                                                            ));
        ImGui.set_function("IsWindowHovered"        , sol::overload(
                                                                sol::resolve<bool()>(IsWindowHovered),
                                                                sol::resolve<bool(int)>(IsWindowHovered)
                                                            ));
        ImGui.set_function("GetWindowDrawList"      , GetWindowDrawList);
        ImGui.set_function("GetWindowPos"           , GetWindowPos);
        ImGui.set_function("GetWindowSize"          , GetWindowSize);
        ImGui.set_function("GetWindowWidth"         , GetWindowWidth);
        ImGui.set_function("GetWindowHeight"        , GetWindowHeight);

        // Prefer  SetNext...
        ImGui.set_function("SetNextWindowPos"        , sol::overload(
                                                                sol::resolve<void(float, float)>(SetNextWindowPos),
                                                                sol::resolve<void(float, float, int)>(SetNextWindowPos),
                                                                sol::resolve<void(float, float, int, float, float)>(SetNextWindowPos)
                                                            ));
        ImGui.set_function("SetNextWindowSize"        , sol::overload(
                                                                sol::resolve<void(float, float)>(SetNextWindowSize),
                                                                sol::resolve<void(float, float, int)>(SetNextWindowSize)
                                                            ));
        ImGui.set_function("SetNextWindowSizeConstraints"  , SetNextWindowSizeConstraints);
        ImGui.set_function("SetNextWindowContentSize"    , SetNextWindowContentSize);
        ImGui.set_function("SetNextWindowCollapsed"      , sol::overload(
                                                                sol::resolve<void(bool)>(SetNextWindowCollapsed),
                                                                sol::resolve<void(bool, int)>(SetNextWindowCollapsed)
                                                            ));
        ImGui.set_function("SetNextWindowFocus"        , SetNextWindowFocus);
        ImGui.set_function("SetNextWindowBgAlpha"      , SetNextWindowBgAlpha);
        ImGui.set_function("SetWindowPos"          , sol::overload(
                                                                sol::resolve<void(float, float)>(SetWindowPos),
                                                                sol::resolve<void(float, float, int)>(SetWindowPos),
                                                                sol::resolve<void(const std::string&, float, float)>(SetWindowPos),
                                                                sol::resolve<void(const std::string&, float, float, int)>(SetWindowPos)
                                                            ));
        ImGui.set_function("SetWindowSize"          , sol::overload(
                                                                sol::resolve<void(float, float)>(SetWindowSize),
                                                                sol::resolve<void(float, float, int)>(SetWindowSize),
                                                                sol::resolve<void(const std::string&, float, float)>(SetWindowSize),
                                                                sol::resolve<void(const std::string&, float, float, int)>(SetWindowSize)
                                                            ));
        ImGui.set_function("SetWindowCollapsed"        , sol::overload(
                                                                sol::resolve<void(bool)>(SetWindowCollapsed),
                                                                sol::resolve<void(bool, int)>(SetWindowCollapsed),
                                                                sol::resolve<void(const std::string&, bool)>(SetWindowCollapsed),
                                                                sol::resolve<void(const std::string&, bool, int)>(SetWindowCollapsed)
                                                            ));
        ImGui.set_function("SetWindowFocus"          , sol::overload(
                                                                sol::resolve<void()>(SetWindowFocus),
                                                                sol::resolve<void(const std::string&)>(SetWindowFocus)
                                                            ));
        ImGui.set_function("SetWindowFontScale"        , SetWindowFontScale);
#pragma endregion Window Utilities

#pragma region Content Region
        ImGui.set_function("GetContentRegionMax"          , GetContentRegionMax);
        ImGui.set_function("GetContentRegionAvail"        , GetContentRegionAvail);
        ImGui.set_function("GetWindowContentRegionMin"    , GetWindowContentRegionMin);
        ImGui.set_function("GetWindowContentRegionMax"    , GetWindowContentRegionMax);
        ImGui.set_function("GetWindowContentRegionWidth"  , GetWindowContentRegionWidth);
#pragma endregion Content Region

#pragma region Windows Scrolling
        ImGui.set_function("GetScrollX"              , GetScrollX);
        ImGui.set_function("GetScrollY"              , GetScrollY);
        ImGui.set_function("GetScrollMaxX"           , GetScrollMaxX);
        ImGui.set_function("GetScrollMaxY"           , GetScrollMaxY);
        ImGui.set_function("SetScrollX"              , SetScrollX);
        ImGui.set_function("SetScrollY"              , SetScrollY);
        ImGui.set_function("SetScrollHereX"          , sol::overload(
                                                                sol::resolve<void()>(SetScrollHereX),
                                                                sol::resolve<void(float)>(SetScrollHereX)
                                                            ));
        ImGui.set_function("SetScrollHereY"          , sol::overload(
                                                                sol::resolve<void()>(SetScrollHereY),
                                                                sol::resolve<void(float)>(SetScrollHereY)
                                                            ));
        ImGui.set_function("SetScrollFromPosX"        , sol::overload(
                                                                sol::resolve<void(float)>(SetScrollFromPosX),
                                                                sol::resolve<void(float, float)>(SetScrollFromPosX)
                                                            ));
        ImGui.set_function("SetScrollFromPosY"        , sol::overload(
                                                                sol::resolve<void(float)>(SetScrollFromPosY),
                                                                sol::resolve<void(float, float)>(SetScrollFromPosY)
                                                            ));
#pragma endregion Windows Scrolling

#pragma region Parameters stacks (shared)
#ifdef SOL_IMGUI_ENABLE_FONT_MANIPULATORS
        ImGui.set_function("PushFont"            , PushFont);
        ImGui.set_function("PopFont"             , PopFont);
#endif // SOL_IMGUI_ENABLE_FONT_MANIPULATORS
        ImGui.set_function("PushStyleColor"         , sol::overload(
                                                                sol::resolve<void(int, int)>(PushStyleColor),
                                                                sol::resolve<void(int, float, float, float, float)>(PushStyleColor)
                                                            ));
        ImGui.set_function("PopStyleColor"          , sol::overload(
                                                                sol::resolve<void()>(PopStyleColor),
                                                                sol::resolve<void(int)>(PopStyleColor)
                                                            ));
        ImGui.set_function("PushStyleVar"           , sol::overload(
                                                                sol::resolve<void(int, float)>(PushStyleVar),
                                                                sol::resolve<void(int, float, float)>(PushStyleVar)
                                                            ));
        ImGui.set_function("PopStyleVar"            , sol::overload(
                                                                sol::resolve<void()>(PopStyleVar),
                                                                sol::resolve<void(int)>(PopStyleVar)
                                                            ));
        ImGui.set_function("GetStyleColorVec4"      , GetStyleColorVec4);
#ifdef SOL_IMGUI_ENABLE_FONT_MANIPULATORS
        ImGui.set_function("GetFont"                , GetFont);
#endif // SOL_IMGUI_ENABLE_FONT_MANIPULATORS
        ImGui.set_function("GetFontSize"            , GetFontSize);
        ImGui.set_function("GetFontTexUvWhitePixel" , GetFontTexUvWhitePixel);
        ImGui.set_function("GetColorU32"            , sol::overload(
                                                                sol::resolve<int(int, float)>(GetColorU32),
                                                                sol::resolve<int(float, float, float, float)>(GetColorU32),
                                                                sol::resolve<int(int)>(GetColorU32)
                                                            ));
#pragma endregion Parameters stacks (shared)

#pragma region Parameters stacks (current window)
        ImGui.set_function("PushItemWidth"           , PushItemWidth);
        ImGui.set_function("PopItemWidth"            , PopItemWidth);
        ImGui.set_function("SetNextItemWidth"        , SetNextItemWidth);
        ImGui.set_function("CalcItemWidth"           , CalcItemWidth);
        ImGui.set_function("PushTextWrapPos"         , sol::overload(
                                                                    sol::resolve<void()>(PushTextWrapPos),
                                                                    sol::resolve<void(float)>(PushTextWrapPos)
                                                            ));
        ImGui.set_function("PopTextWrapPos"          , PopTextWrapPos);
        ImGui.set_function("PushAllowKeyboardFocus"  , PushAllowKeyboardFocus);
        ImGui.set_function("PopAllowKeyboardFocus"   , PopAllowKeyboardFocus);
        ImGui.set_function("PushButtonRepeat"        , PushButtonRepeat);
        ImGui.set_function("PopButtonRepeat"         , PopButtonRepeat);
#pragma endregion Parameters stacks (current window)

#pragma region Cursor / Layout
        ImGui.set_function("Separator"               , Separator);
        ImGui.set_function("SameLine"                , sol::overload(
                                                                sol::resolve<void()>(SameLine),
                                                                sol::resolve<void(float)>(SameLine)
                                                            ));
        ImGui.set_function("NewLine"                 , NewLine);
        ImGui.set_function("Spacing"                 , Spacing);
        ImGui.set_function("Dummy"                   , Dummy);
        ImGui.set_function("Indent"                  , sol::overload(
                                                                sol::resolve<void()>(Indent),
                                                                sol::resolve<void(float)>(Indent)
                                                            ));
        ImGui.set_function("Unindent"                , sol::overload(
                                                                sol::resolve<void()>(Unindent),
                                                                sol::resolve<void(float)>(Unindent)
                                                            ));
        ImGui.set_function("BeginGroup"                   , BeginGroup);
        ImGui.set_function("EndGroup"                     , EndGroup);
        ImGui.set_function("GetCursorPos"                 , GetCursorPos);
        ImGui.set_function("GetCursorPosX"                , GetCursorPosX);
        ImGui.set_function("GetCursorPosY"                , GetCursorPosY);
        ImGui.set_function("SetCursorPos"                 , SetCursorPos);
        ImGui.set_function("SetCursorPosX"                , SetCursorPosX);
        ImGui.set_function("SetCursorPosY"                , SetCursorPosY);
        ImGui.set_function("GetCursorStartPos"            , GetCursorStartPos);
        ImGui.set_function("GetCursorScreenPos"           , GetCursorScreenPos);
        ImGui.set_function("SetCursorScreenPos"           , SetCursorScreenPos);
        ImGui.set_function("AlignTextToFramePadding"      , AlignTextToFramePadding);
        ImGui.set_function("GetTextLineHeight"            , GetTextLineHeight);
        ImGui.set_function("GetTextLineHeightWithSpacing" , GetTextLineHeightWithSpacing);
        ImGui.set_function("GetFrameHeight"               , GetFrameHeight);
        ImGui.set_function("GetFrameHeightWithSpacing"    , GetFrameHeightWithSpacing);
#pragma endregion Cursor / Layout

#pragma region ID stack / scopes
        ImGui.set_function("PushID"              , sol::overload(
                                                                sol::resolve<void(const std::string&)>(PushID),
                                                                sol::resolve<void(int)>(PushID)
                                                            ));
        ImGui.set_function("PopID"              , PopID);
        ImGui.set_function("GetID"              , GetID);
#pragma endregion ID stack / scopes

#pragma region Widgets: Text
        ImGui.set_function("TextUnformatted"        , TextUnformatted);
        ImGui.set_function("Text"                 , Text);
        ImGui.set_function("TextColored"          , TextColored);
        ImGui.set_function("TextDisabled"         , TextDisabled);
        ImGui.set_function("TextWrapped"          , TextWrapped);
        ImGui.set_function("LabelText"            , LabelText);
        ImGui.set_function("BulletText"           , BulletText);
#pragma endregion Widgets: Text

#pragma region Widgets: Main
        ImGui.set_function("Button"              , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(Button),
                                                                sol::resolve<bool(const std::string&, float, float)>(Button)
                                                            ));
        ImGui.set_function("SmallButton"            , SmallButton);
        ImGui.set_function("InvisibleButton"        , InvisibleButton);
        ImGui.set_function("ArrowButton"            , ArrowButton);
        ImGui.set_function("Checkbox"               , Checkbox);
        ImGui.set_function("RadioButton"            , sol::overload(
                                                                sol::resolve<bool(const std::string&, bool)>(RadioButton),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int)>(RadioButton)
                                                            ));
        ImGui.set_function("ProgressBar"          , sol::overload(
                                                                sol::resolve<void(float)>(ProgressBar),
                                                                sol::resolve<void(float, float, float)>(ProgressBar),
                                                                sol::resolve<void(float, float, float, const std::string&)>(ProgressBar)
                                                            ));
        ImGui.set_function("Bullet"              , Bullet);
#pragma endregion Widgets: Main

#pragma region Widgets: Combo Box
        ImGui.set_function("BeginCombo"          , sol::overload(
                                                                sol::resolve<bool(const std::string&, const std::string&)>(BeginCombo),
                                                                sol::resolve<bool(const std::string&, const std::string&, int)>(BeginCombo)
                                                            ));
        ImGui.set_function("EndCombo"            , EndCombo);
        ImGui.set_function("Combo"               , sol::overload(
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int)>(Combo),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int, int)>(Combo),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, const std::string&)>(Combo),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, const std::string&, int)>(Combo)
                                                            ));
#pragma endregion Widgets: Combo Box

#pragma region Widgets: Drags
        ImGui.set_function("DragFloat"            , sol::overload(
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float)>(DragFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(DragFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(DragFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float)>(DragFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, const std::string&)>(DragFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, const std::string&, int)>(DragFloat)
                                                            ));
        ImGui.set_function("DragFloat2"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(DragFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float)>(DragFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float)>(DragFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float)>(DragFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float, const std::string&)>(DragFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float, const std::string&, int)>(DragFloat2)
                                                            ));
        ImGui.set_function("DragFloat3"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(DragFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float)>(DragFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float)>(DragFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float)>(DragFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float, const std::string&)>(DragFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float, const std::string&, int)>(DragFloat3)
                                                            ));
        ImGui.set_function("DragFloat4"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(DragFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float)>(DragFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float)>(DragFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float)>(DragFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float, const std::string&)>(DragFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, float, const std::string&, int)>(DragFloat4)
                                                            ));
        ImGui.set_function("DragInt"            , sol::overload(
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int)>(DragInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, float)>(DragInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int)>(DragInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int, int)>(DragInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int, int, const std::string&)>(DragInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, float, int, int, const std::string&, int)>(DragInt)
                                                            ));
        ImGui.set_function("DragInt2"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(DragInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float)>(DragInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int)>(DragInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int)>(DragInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int, const std::string&)>(DragInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int, const std::string&, int)>(DragInt2)
                                                            ));
        ImGui.set_function("DragInt3"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(DragInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float)>(DragInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int)>(DragInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int)>(DragInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int, const std::string&)>(DragInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int, const std::string&, int)>(DragInt3)
                                                            ));
        ImGui.set_function("DragInt4"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(DragInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float)>(DragInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int)>(DragInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int)>(DragInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int, const std::string&)>(DragInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, float, int, int, const std::string&, int)>(DragInt4)
                                                            ));
#pragma endregion Widgets: Drags

#pragma region Widgets: Sliders
        ImGui.set_function("SliderFloat"          , sol::overload(
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(SliderFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&)>(SliderFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&, int)>(SliderFloat)
                                                            ));
        ImGui.set_function("SliderFloat2"          , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float)>(SliderFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, const std::string&)>(SliderFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, const std::string&, int)>(SliderFloat2)
                                                            ));
        ImGui.set_function("SliderFloat3"          , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float)>(SliderFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, const std::string&)>(SliderFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, const std::string&, int)>(SliderFloat3)
                                                            ));
        ImGui.set_function("SliderFloat4"          , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float)>(SliderFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, const std::string&)>(SliderFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, float, float, const std::string&, int)>(SliderFloat4)
                                                            ));
        ImGui.set_function("SliderAngle"          , sol::overload(
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float)>(SliderAngle),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(SliderAngle),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(SliderAngle),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&)>(SliderAngle),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&, int)>(SliderAngle)
                                                            ));
        ImGui.set_function("SliderInt"            , sol::overload(
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int)>(SliderInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int, const std::string&)>(SliderInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int, const std::string&, int)>(SliderInt)
                                                            ));
        ImGui.set_function("SliderInt2"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int)>(SliderInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int, const std::string&)>(SliderInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int, const std::string&, int)>(SliderInt2)
                                                            ));
        ImGui.set_function("SliderInt3"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int)>(SliderInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int, const std::string&)>(SliderInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int, const std::string&, int)>(SliderInt3)
                                                            ));
        ImGui.set_function("SliderInt4"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int)>(SliderInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int, const std::string&)>(SliderInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int, int, const std::string&, int)>(SliderInt4)
                                                            ));
        ImGui.set_function("VSliderFloat"          , sol::overload(
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, float)>(VSliderFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, float, const std::string&)>(VSliderFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, float, float, const std::string&, int)>(VSliderFloat)
                                                            ));
        ImGui.set_function("VSliderInt"            , sol::overload(
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, float, float, int, int, int)>(VSliderInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, float, float, int, int, int, const std::string&)>(VSliderInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, float, float, int, int, int, const std::string&, int)>(VSliderInt)
                                                            ));
#pragma endregion Widgets: Sliders

#pragma region Widgets: Inputs using Keyboard
        ImGui.set_function("InputText"            , sol::overload(
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int)>(InputText),
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int, int)>(InputText)
                                                            ));
        ImGui.set_function("InputTextMultiline"        , sol::overload(
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int)>(InputTextMultiline),
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int, float, float)>(InputTextMultiline),
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, std::string, unsigned int, float, float, int)>(InputTextMultiline)
                                                            ));
        ImGui.set_function("InputTextWithHint"        , sol::overload(
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, const std::string&, std::string, unsigned int)>(InputTextWithHint),
                                                                sol::resolve<std::tuple<std::string, bool>(const std::string&, const std::string&, std::string, unsigned int, int)>(InputTextWithHint)
                                                            ));
        ImGui.set_function("InputFloat"            , sol::overload(
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float)>(InputFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float)>(InputFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float)>(InputFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&)>(InputFloat),
                                                                sol::resolve<std::tuple<float, bool>(const std::string&, float, float, float, const std::string&, int)>(InputFloat)
                                                            ));
        ImGui.set_function("InputFloat2"          , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(InputFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, const std::string&)>(InputFloat2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, const std::string&, int)>(InputFloat2)
                                                            ));
        ImGui.set_function("InputFloat3"          , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(InputFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, const std::string&)>(InputFloat3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, const std::string&, int)>(InputFloat3)
                                                            ));
        ImGui.set_function("InputFloat4"          , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(InputFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, const std::string&)>(InputFloat4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, const std::string&, int)>(InputFloat4)
                                                            ));
        ImGui.set_function("InputInt"            , sol::overload(
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int)>(InputInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int)>(InputInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int)>(InputInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int)>(InputInt),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, int, int, int)>(InputInt)
                                                            ));
        ImGui.set_function("InputInt2"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(InputInt2),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int)>(InputInt2)
                                                            ));
        ImGui.set_function("InputInt3"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(InputInt3),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int)>(InputInt3)
                                                            ));
        ImGui.set_function("InputInt4"            , sol::overload(
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&)>(InputInt4),
                                                                sol::resolve<std::tuple<sol::as_table_t<std::vector<int>>, bool>(const std::string&, const sol::table&, int)>(InputInt4)
                                                            ));
        ImGui.set_function("InputDouble"          , sol::overload(
                                                                sol::resolve<std::tuple<double, bool>(const std::string&, double)>(InputDouble),
                                                                sol::resolve<std::tuple<double, bool>(const std::string&, double, double)>(InputDouble),
                                                                sol::resolve<std::tuple<double, bool>(const std::string&, double, double, double)>(InputDouble),
                                                                sol::resolve<std::tuple<double, bool>(const std::string&, double, double, double, const std::string&)>(InputDouble),
                                                                sol::resolve<std::tuple<double, bool>(const std::string&, double, double, double, const std::string&, int)>(InputDouble)
                                                            ));
#pragma endregion Widgets: Inputs using Keyboard

#pragma region Widgets: Color Editor / Picker
        ImGui.set_function("ColorEdit3"            , sol::overload(
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorEdit3),
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorEdit3)
                                                            ));
        ImGui.set_function("ColorEdit4"            , sol::overload(
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorEdit4),
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorEdit4)
                                                            ));
        ImGui.set_function("ColorPicker3"          , sol::overload(
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorPicker3),
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorPicker3)
                                                            ));
        ImGui.set_function("ColorPicker4"          , sol::overload(
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&)>(ColorPicker4),
                                                                sol::resolve<std::tuple <sol::as_table_t<std::vector<float>>, bool>(const std::string&, const sol::table&, int)>(ColorPicker4)
                                                            ));
        ImGui.set_function("ColorButton"           , sol::overload(
                                                                sol::resolve<bool(const std::string&, const sol::table&)>(ColorButton),
                                                                sol::resolve<bool(const std::string&, const sol::table&, int)>(ColorButton),
                                                                sol::resolve<bool(const std::string&, const sol::table&, int, float, float)>(ColorButton)
                                                            ));
#pragma endregion Widgets: Color Editor / Picker

#pragma region Widgets: Trees
        ImGui.set_function("TreeNode"            , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(TreeNode),
                                                                sol::resolve<bool(const std::string&, const std::string&)>(TreeNode)
                                                            ));
        ImGui.set_function("TreeNodeEx"            , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(TreeNodeEx),
                                                                sol::resolve<bool(const std::string&, int)>(TreeNodeEx),
                                                                sol::resolve<bool(const std::string&, int, const std::string&)>(TreeNodeEx)
                                                            ));
        ImGui.set_function("TreePush"            , TreePush);
        ImGui.set_function("GetTreeNodeToLabelSpacing"    , GetTreeNodeToLabelSpacing);
        ImGui.set_function("CollapsingHeader"        , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(CollapsingHeader),
                                                                sol::resolve<bool(const std::string&, int)>(CollapsingHeader),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(CollapsingHeader),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int)>(CollapsingHeader)
                                                            ));
        ImGui.set_function("SetNextItemOpen"        , sol::overload(
                                                                sol::resolve<void(bool)>(SetNextItemOpen),
                                                                sol::resolve<void(bool, int)>(SetNextItemOpen)
                                                            ));
#pragma endregion Widgets: Trees

#pragma region Widgets: Selectables
        ImGui.set_function("Selectable"            , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(Selectable),
                                                                sol::resolve<bool(const std::string&, bool)>(Selectable),
                                                                sol::resolve<bool(const std::string&, bool, int)>(Selectable),
                                                                sol::resolve<bool(const std::string&, bool, int, float, float)>(Selectable)
                                                            ));
#pragma endregion Widgets: Selectables

#pragma region Widgets: List Boxes
        ImGui.set_function("ListBox"            , sol::overload(
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int)>(ListBox),
                                                                sol::resolve<std::tuple<int, bool>(const std::string&, int, const sol::table&, int, int)>(ListBox)
                                                            ));
        ImGui.set_function("BeginListBox"       , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginListBox),
                                                                sol::resolve<bool(const std::string&, float, float)>(BeginListBox)
                                                            ));
        ImGui.set_function("EndListBox"         , EndListBox);
#pragma endregion Widgets: List Boxes

#pragma region Widgets: Value() Helpers
        ImGui.set_function("Value"              , sol::overload(
                                                                sol::resolve<void(const std::string&, bool)>(Value),
                                                                sol::resolve<void(const std::string&, int)>(Value),
                                                                sol::resolve<void(const std::string&, unsigned int)>(Value),
                                                                sol::resolve<void(const std::string&, float)>(Value),
                                                                sol::resolve<void(const std::string&, float, const std::string&)>(Value)
                                                            ));
#pragma endregion Widgets: Value() Helpers

#pragma region Widgets: Menu
        ImGui.set_function("BeginMenuBar"            , BeginMenuBar);
        ImGui.set_function("EndMenuBar"              , EndMenuBar);
        ImGui.set_function("BeginMainMenuBar"        , BeginMainMenuBar);
        ImGui.set_function("EndMainMenuBar"          , EndMainMenuBar);
        ImGui.set_function("BeginMenu"               , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginMenu),
                                                                sol::resolve<bool(const std::string&, bool)>(BeginMenu)
                                                            ));
        ImGui.set_function("EndMenu"             , EndMenu);
        ImGui.set_function("MenuItem"            , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(MenuItem),
                                                                sol::resolve<bool(const std::string&, const std::string&)>(MenuItem),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, const std::string&, bool)>(MenuItem),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, const std::string&, bool, bool)>(MenuItem)
                                                            ));
#pragma endregion Widgets: Menu

#pragma region Tooltips
        ImGui.set_function("BeginTooltip"          , BeginTooltip);
        ImGui.set_function("EndTooltip"            , EndTooltip);
        ImGui.set_function("SetTooltip"            , SetTooltip);
#pragma endregion Tooltips

#pragma region Popups, Modals
        ImGui.set_function("BeginPopup"            , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginPopup),
                                                                sol::resolve<bool(const std::string&, int)>(BeginPopup)
                                                            ));
        ImGui.set_function("BeginPopupModal"        , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginPopupModal),
                                                                sol::resolve<bool(const std::string&, int)>(BeginPopupModal),
                                                                sol::resolve<bool(const std::string&, bool)>(BeginPopupModal),
                                                                sol::resolve<bool(const std::string&, bool, int)>(BeginPopupModal)
                                                            ));
        ImGui.set_function("EndPopup"             , EndPopup);
        ImGui.set_function("OpenPopup"            , sol::overload(
                                                                sol::resolve<void(const std::string&)>(OpenPopup),
                                                                sol::resolve<void(const std::string&, int)>(OpenPopup)
                                                            ));
        ImGui.set_function("CloseCurrentPopup"          , CloseCurrentPopup);
        ImGui.set_function("BeginPopupContextItem"      , sol::overload(
                                                                sol::resolve<bool()>(BeginPopupContextItem),
                                                                sol::resolve<bool(const std::string&)>(BeginPopupContextItem),
                                                                sol::resolve<bool(const std::string&, int)>(BeginPopupContextItem)
                                                            ));
        ImGui.set_function("BeginPopupContextWindow"    , sol::overload(
                                                                sol::resolve<bool()>(BeginPopupContextWindow),
                                                                sol::resolve<bool(const std::string&)>(BeginPopupContextWindow),
                                                                sol::resolve<bool(const std::string&, int)>(BeginPopupContextWindow)
                                                            ));
        ImGui.set_function("BeginPopupContextVoid"      , sol::overload(
                                                                sol::resolve<bool()>(BeginPopupContextVoid),
                                                                sol::resolve<bool(const std::string&)>(BeginPopupContextVoid),
                                                                sol::resolve<bool(const std::string&, int)>(BeginPopupContextVoid)
                                                            ));
        ImGui.set_function("IsPopupOpen"          , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(IsPopupOpen),
                                                                sol::resolve<bool(const std::string&, int)>(IsPopupOpen)
                                                            ));
#pragma endregion Popups, Modals

#pragma region Tables
        ImGui.set_function("BeginTable"              , sol::overload(
                                                                      sol::resolve<bool(const std::string&, int)>(BeginTable),
                                                                      sol::resolve<bool(const std::string&, int, int)>(BeginTable),
                                                                      sol::resolve<bool(const std::string&, int, int, float, float)>(BeginTable),
                                                                      sol::resolve<bool(const std::string&, int, int, float, float, float)>(BeginTable)
                                                                    ));
        ImGui.set_function("EndTable"                , EndTable);
        ImGui.set_function("TableNextRow"            , sol::overload(
                                                                      sol::resolve<void()>(TableNextRow),
                                                                      sol::resolve<void(int)>(TableNextRow),
                                                                      sol::resolve<void(int, float)>(TableNextRow)
                                                                    ));
        ImGui.set_function("TableNextColumn"         , TableNextColumn);
        ImGui.set_function("TableSetColumnIndex"     , TableSetColumnIndex);
        ImGui.set_function("TableSetupColumn"        , sol::overload(
                                                                      sol::resolve<void(const std::string&)>(TableSetupColumn),
                                                                      sol::resolve<void(const std::string&, int)>(TableSetupColumn),
                                                                      sol::resolve<void(const std::string&, int, float)>(TableSetupColumn),
                                                                      sol::resolve<void(const std::string&, int, float, int)>(TableSetupColumn)
                                                                    ));
        ImGui.set_function("TableSetupScrollFreeze"  , TableSetupScrollFreeze);
        ImGui.set_function("TableHeadersRow"         , TableHeadersRow);
        ImGui.set_function("TableHeader"             , TableHeader);
        ImGui.set_function("TableGetSortSpecs"       , TableGetSortSpecs);
        ImGui.set_function("TableGetColumnCount"     , TableGetColumnCount);
        ImGui.set_function("TableGetColumnIndex"     , TableGetColumnIndex);
        ImGui.set_function("TableGetRowIndex"        , TableGetRowIndex);
        ImGui.set_function("TableGetColumnName"      , sol::overload(
                                                                      sol::resolve<std::string()>(TableGetColumnName),
                                                                      sol::resolve<std::string(int)>(TableGetColumnName)
                                                                    ));
        ImGui.set_function("TableGetColumnFlags"     , sol::overload(
                                                                      sol::resolve<int()>(TableGetColumnFlags),
                                                                      sol::resolve<int(int)>(TableGetColumnFlags)
                                                                    ));
        ImGui.set_function("TableSetBgColor"         , sol::overload(
                                                                      sol::resolve<void(int, int)>(TableSetBgColor),
                                                                      sol::resolve<void(int, float, float, float, float)>(TableSetBgColor),
                                                                      sol::resolve<void(int, int, int)>(TableSetBgColor),
                                                                      sol::resolve<void(int, float, float, float, float, int)>(TableSetBgColor)
                                                                    ));
#pragma endregion Tables

#pragma region Columns
        ImGui.set_function("Columns"            , sol::overload(
                                                                sol::resolve<void()>(Columns),
                                                                sol::resolve<void(int)>(Columns),
                                                                sol::resolve<void(int, const std::string&)>(Columns),
                                                                sol::resolve<void(int, const std::string&, bool)>(Columns)
                                                            ));
        ImGui.set_function("NextColumn"              , NextColumn);
        ImGui.set_function("GetColumnIndex"          , GetColumnIndex);
        ImGui.set_function("GetColumnWidth"          , sol::overload(
                                                                sol::resolve<float()>(GetColumnWidth),
                                                                sol::resolve<float(int)>(GetColumnWidth)
                                                            ));
        ImGui.set_function("SetColumnWidth"          , SetColumnWidth);
        ImGui.set_function("GetColumnOffset"         , sol::overload(
                                                                sol::resolve<float()>(GetColumnOffset),
                                                                sol::resolve<float(int)>(GetColumnOffset)
                                                            ));
        ImGui.set_function("SetColumnOffset"        , SetColumnOffset);
        ImGui.set_function("GetColumnsCount"        , GetColumnsCount);
#pragma endregion Columns

#pragma region Tab Bars, Tabs
        ImGui.set_function("BeginTabBar"          , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginTabBar),
                                                                sol::resolve<bool(const std::string&, int)>(BeginTabBar)
                                                            ));
        ImGui.set_function("EndTabBar"             , EndTabBar);
        ImGui.set_function("BeginTabItem"          , sol::overload(
                                                                sol::resolve<bool(const std::string&)>(BeginTabItem),
                                                                sol::resolve<bool(const std::string&, int)>(BeginTabItem),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, bool)>(BeginTabItem),
                                                                sol::resolve<std::tuple<bool, bool>(const std::string&, bool, int)>(BeginTabItem)
                                                            ));
        ImGui.set_function("EndTabItem"              , EndTabItem);
        ImGui.set_function("SetTabItemClosed"        , SetTabItemClosed);
#pragma endregion Tab Bars, Tabs

#pragma region Clipping
        ImGui.set_function("PushClipRect"           , PushClipRect);
        ImGui.set_function("PopClipRect"            , PopClipRect);
#pragma endregion Clipping

#pragma region Focus, Activation
        ImGui.set_function("SetItemDefaultFocus"       , SetItemDefaultFocus);
        ImGui.set_function("SetKeyboardFocusHere"      , sol::overload(
                                                                sol::resolve<void()>(SetKeyboardFocusHere),
                                                                sol::resolve<void(int)>(SetKeyboardFocusHere)
                                                            ));
#pragma endregion Focus, Activation

#pragma region Item/Widgets Utilities
        ImGui.set_function("IsItemHovered"          , sol::overload(
                                                                sol::resolve<bool()>(IsItemHovered),
                                                                sol::resolve<bool(int)>(IsItemHovered)
                                                            ));
        ImGui.set_function("IsItemActive"           , IsItemActive);
        ImGui.set_function("IsItemFocused"          , IsItemFocused);
        ImGui.set_function("IsItemClicked"          , sol::overload(
                                                                sol::resolve<bool()>(IsItemClicked),
                                                                sol::resolve<bool(int)>(IsItemClicked)
                                                            ));
        ImGui.set_function("IsItemVisible"              , IsItemVisible);
        ImGui.set_function("IsItemEdited"               , IsItemEdited);
        ImGui.set_function("IsItemActivated"            , IsItemActivated);
        ImGui.set_function("IsItemDeactivated"          , IsItemDeactivated);
        ImGui.set_function("IsItemDeactivatedAfterEdit" , IsItemDeactivatedAfterEdit);
        ImGui.set_function("IsItemToggledOpen"          , IsItemToggledOpen);
        ImGui.set_function("IsAnyItemHovered"           , IsAnyItemHovered);
        ImGui.set_function("IsAnyItemActive"            , IsAnyItemActive);
        ImGui.set_function("IsAnyItemFocused"           , IsAnyItemFocused);
        ImGui.set_function("GetItemRectMin"             , GetItemRectMin);
        ImGui.set_function("GetItemRectMax"             , GetItemRectMax);
        ImGui.set_function("GetItemRectSize"            , GetItemRectSize);
        ImGui.set_function("SetItemAllowOverlap"        , SetItemAllowOverlap);
#pragma endregion Item/Widgets Utilities

#pragma region Miscellaneous Utilities
        ImGui.set_function("IsRectVisible"          , sol::overload(
                                                                sol::resolve<bool(float, float)>(IsRectVisible),
                                                                sol::resolve<bool(float, float, float, float)>(IsRectVisible)
                                                            ));
        ImGui.set_function("GetTime"                , GetTime);
        ImGui.set_function("GetFrameCount"          , GetFrameCount);
        ImGui.set_function("GetBackgroundDrawList"  , GetBackgroundDrawList);
        ImGui.set_function("GetForegroundDrawList"  , GetForegroundDrawList);
        ImGui.set_function("GetStyleColorName"      , GetStyleColorName);
        ImGui.set_function("BeginChildFrame"        , sol::overload(
                                                                sol::resolve<bool(unsigned int, float, float)>(BeginChildFrame),
                                                                sol::resolve<bool(unsigned int, float, float, int)>(BeginChildFrame)
                                                            ));
        ImGui.set_function("EndChildFrame"          , EndChildFrame);
#pragma endregion Miscellaneous Utilities

#pragma region Text Utilities
        ImGui.set_function("CalcTextSize"          , sol::overload(
                                                                sol::resolve<std::tuple<float, float>(const std::string&)>(CalcTextSize),
                                                                sol::resolve<std::tuple<float, float>(const std::string&, bool)>(CalcTextSize),
                                                                sol::resolve<std::tuple<float, float>(const std::string&, bool, float)>(CalcTextSize)
                                                            ));
#pragma endregion Text Utilities

#pragma region Color Utilities
        ImGui.set_function("ColorConvertU32ToFloat4"     , ColorConvertU32ToFloat4);
        ImGui.set_function("ColorConvertFloat4ToU32"     , ColorConvertFloat4ToU32);
        ImGui.set_function("ColorConvertRGBtoHSV"        , ColorConvertRGBtoHSV);
        ImGui.set_function("ColorConvertHSVtoRGB"        , ColorConvertHSVtoRGB);
#pragma endregion Color Utilities

#pragma region Inputs Utilities: Mouse
        ImGui.set_function("IsMouseHoveringRect"       , sol::overload(
                                                                sol::resolve<bool(float, float, float, float)>(IsMouseHoveringRect),
                                                                sol::resolve<bool(float, float, float, float, bool)>(IsMouseHoveringRect)
                                                            ));
        ImGui.set_function("GetMousePos"              , GetMousePos);
        ImGui.set_function("GetMousePosOnOpeningCurrentPopup", GetMousePosOnOpeningCurrentPopup);
        ImGui.set_function("IsMouseDragging"          , sol::overload(
                                                                sol::resolve<bool(int)>(IsMouseDragging),
                                                                sol::resolve<bool(int, float)>(IsMouseDragging)
                                                            ));
        ImGui.set_function("GetMouseDragDelta"        , sol::overload(
                                                                sol::resolve<std::tuple<float, float>()>(GetMouseDragDelta),
                                                                sol::resolve<std::tuple<float, float>(int)>(GetMouseDragDelta),
                                                                sol::resolve<std::tuple<float, float>(int, float)>(GetMouseDragDelta)
                                                            ));
        ImGui.set_function("ResetMouseDragDelta"      , sol::overload(
                                                                sol::resolve<void()>(ResetMouseDragDelta),
                                                                sol::resolve<void(int)>(ResetMouseDragDelta)
                                                            ));
#pragma endregion Inputs Utilities: Mouse

#pragma region Clipboard Utilities
        ImGui.set_function("GetClipboardText"        , GetClipboardText);
        ImGui.set_function("SetClipboardText"        , SetClipboardText);
#pragma endregion Clipboard Utilities

#pragma region Drawing APIs
        ImGui.set_function("ImDrawListAddLine"                 , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int)>(ImDrawListAddLine),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int, float)>(ImDrawListAddLine)
                                                                    ));
        ImGui.set_function("ImDrawListAddRect"                 , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int)>(ImDrawListAddRect),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int, float)>(ImDrawListAddRect),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int, float, int)>(ImDrawListAddRect),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int, float, int, float)>(ImDrawListAddRect)
                                                                    ));
        ImGui.set_function("ImDrawListAddRectFilled"           , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int)>(ImDrawListAddRectFilled),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int, float)>(ImDrawListAddRectFilled),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, int, float, int)>(ImDrawListAddRectFilled)
                                                                    ));
        ImGui.set_function("ImDrawListAddRectFilledMultiColor" , ImDrawListAddRectFilledMultiColor);
        ImGui.set_function("ImDrawListAddQuad"                 , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, float, float, int)>(ImDrawListAddQuad),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, float, float, int, float)>(ImDrawListAddQuad)
                                                                    ));
        ImGui.set_function("ImDrawListAddQuadFilled"           , ImDrawListAddQuadFilled);
        ImGui.set_function("ImDrawListAddTriangle"             , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, int)>(ImDrawListAddTriangle),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, int, float)>(ImDrawListAddTriangle)
                                                                    ));
        ImGui.set_function("ImDrawListAddTriangleFilled"       , ImDrawListAddTriangleFilled);
        ImGui.set_function("ImDrawListAddCircle"               , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int)>(ImDrawListAddCircle),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, int)>(ImDrawListAddCircle),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, int, float)>(ImDrawListAddCircle)
                                                                    ));
        ImGui.set_function("ImDrawListAddCircleFilled"         , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int)>(ImDrawListAddCircleFilled),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, int)>(ImDrawListAddCircleFilled)
                                                                    ));
        ImGui.set_function("ImDrawListAddNgon"                 , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, int)>(ImDrawListAddNgon),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, int, float)>(ImDrawListAddNgon)
                                                                    ));
        ImGui.set_function("ImDrawListAddNgonFilled"           , ImDrawListAddNgonFilled);
        ImGui.set_function("ImDrawListAddText"                 , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, int, const std::string&)>(ImDrawListAddText),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, const std::string&)>(ImDrawListAddText),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, int, const std::string&, float)>(ImDrawListAddText)
                                                                            //    sol::resolve<void(ImDrawList*, float, float, float, int, const std::string&, float, ImVec4*)>(ImDrawListAddText)
                                                                    ));
        // ImGui.set_function("ImDrawListAddPolyline  "           , ImDrawListAddPolyline);
        // ImGui.set_function("ImDrawListAddConvexPolyFilled"     , ImDrawListAddConvexPolyFilled);
        ImGui.set_function("ImDrawListAddBezierCubic"          , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, float, float, int, float)>(ImDrawListAddBezierCubic),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, float, float, int, float, int)>(ImDrawListAddBezierCubic)
                                                                    ));
        ImGui.set_function("ImDrawListAddBezierQuadratic"      , sol::overload(
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, int, float)>(ImDrawListAddBezierQuadratic),
                                                                               sol::resolve<void(ImDrawList*, float, float, float, float, float, float, int, float, int)>(ImDrawListAddBezierQuadratic)
                                                                    ));
#pragma endregion Drawing APIs
    }
}
