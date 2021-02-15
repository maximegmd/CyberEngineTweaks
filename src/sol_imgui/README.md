# sol2_ImGui_Bindings

## Checkout former repo here: https://github.com/MSeys/sol2_ImGui_Bindings
This version was modified a bit to make it work properly with Cyber Engine Tweaks, docs part of README.md taken and modified to match modifications.
If you want some sample demo, see demo in Dear ImGui master, translates almost 1:1 to Lua: https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp

# How to Use
```cpp
  // Call this function!
  sol_ImGui::InitBindings(lua); // lua being your sol::state
```

# Documentation
You can find all the supported functions and overloads below.

## Windows
```lua
  -- ImGui.Begin(...)
  -- Parameters: text (name), bool (open) [O], ImGuiWindowFlags (flags) [O]
  -- Returns A: bool (shouldDraw)
  -- Returns B & C: bool (open), bool (shouldDraw)
  -- Overloads
  shouldDraw = ImGui.Begin("Name")
  shouldDraw = ImGui.Begin("Name", ImGuiWindowFlags.NoMove)
  open, shouldDraw = ImGui.Begin("Name", open)
  open, shouldDraw = ImGui.Begin("Name", open, ImGuiWindowFlags.NoMove)

  -- ImGui.End()
  ImGui.End()
```

## Child Windows
```lua
  -- ImGui.BeginChild(...)
  -- Parameters: text (name), float (size_x) [O], float (size_y) [O], bool (border) [O], ImGuiWindowFlags (flags) [O]
  -- Returns: bool (shouldDraw)
  -- Overloads
  shouldDraw = ImGui.BeginChild("Name")
  shouldDraw = ImGui.BeginChild("Name", 100)
  shouldDraw = ImGui.BeginChild("Name", 100, 200)
  shouldDraw = ImGui.BeginChild("Name", 100, 200, true)
  shouldDraw = ImGui.BeginChild("Name", 100, 200, true, ImGuiWindowFlags.NoMove)

  -- ImGui.EndChild()
  ImGui.EndChild()
```

## Windows Utilities
```lua
  -- ImGui.IsWindowAppearing()
  -- Returns: bool (appearing)
  appearing = ImGui.IsWindowAppearing()

  -- ImGui.IsWindowCollapsed()
  -- Returns: bool (collapsed)
  collapsed = ImGui.IsWindowCollapsed()

  -- ImGui.IsWindowFocused(...)
  -- Parameters: ImGuiFocusedFlags (flags) [O]
  -- Returns: bool (focused)
  -- Overloads
  focused = ImGui.IsWindowFocused()
  focused = ImGui.IsWindowFocused(ImGuiFocusedFlags.ChildWindows)

  -- ImGui.IsWindowHovered(...)
  -- Parameters: ImGuiHoveredFlags (flags) [O]
  -- Returns: bool (hovered)
  -- Overloads
  hovered = ImGui.IswindowHovered()
  hovered = ImGui.IsWindowHovered(ImGuiHoveredFlags.ChildWindows)

  -- ImGui.GetWindowDpiScale()
  -- Returns: float (dpiScale)
  dpiScale = ImGui.GetWindowDpiScale()

  -- ImGui.GetWindowPos()
  -- Returns: float (pos_x), float (pos_y)
  pos_x, pos_y = ImGui.GetWindowPos()

  -- ImGui.GetWindowSize()
  -- Returns: float (size_x), float (size_y)
  size_x, size_y = ImGui.GetWindowSize()

  -- ImGui.GetWindowWidth()
  -- Returns: float (width)
  width = ImGui.GetWindowWidth()

  -- ImGui.GetWindowHeight()
  -- Returns: float (height)
  height = ImGui.GetWindowHeight()

  -- ImGui.SetNextWindowPos(...)
  -- Parameters: float (pos_x), float (pos_y), ImGuiCond (cond) [O], float (pivot_x) [O], float (pivot_y) [O]
  -- Overloads
  ImGui.SetNextWindowPos(100, 100)
  ImGui.SetNextWindowPos(100, 100, ImGuiCond.Always)
  ImGui.SetNextWindowPos(100, 100, ImGuiCond.Always, 0, 0.5)

  -- ImGui.SetNextWindowSize(...)
  -- Parameters: float (size_x), float (size_y), ImGuiCond (cond) [O]
  -- Overloads
  ImGui.SetNextWindowSize(500, 500)
  ImGui.SetNextWindowSize(500, 500, ImGuiCond.Appearing)

  -- ImGui.SetNextWindowSizeConstraints(...)
  -- Parameters: float (min_x), float (min_y), float (max_x), float (max_y)
  ImGui.SetNextWindowSizeConstraints(100, 100, 500, 600)

  -- ImGui.SetNextWindowContentSize(...)
  -- Parameters: float (size_x), float (size_y)
  ImGui.SetNextWindowContentSize(200, 100)

  -- ImGui.SetNextWindowCollapsed(...)
  -- Parameters: bool (collapsed), ImGuiCond (cond) [O]
  -- Overloads
  ImGui.SetNextWindowCollapsed(true)
  ImGui.SetNextWindowCollapsed(true, ImGuiCond.Appearing)

  -- ImGui.SetNextWindowFocus()
  ImGui.SetNextWindowFocus()

  -- ImGui.SetNextWindowBgAlpha(...)
  -- Parameters: float (alpha)
  ImGui.SetNextWindowBgAlpha(0.5)

  -- ImGui.SetWindowPos(...)
  -- Parameters: float (pos_x), float (pos_y), ImguiCond (cond) [O]
  -- Overloads
  ImGui.SetWindowPos(100, 100)
  ImGui.SetWindowPos(100, 100, ImGuiCond.Appearing)

  -- ImGui.SetWindowSize(...)
  -- Parameters: float (size_x), float (size_y), ImguiCond (cond) [O]
  -- Overloads
  ImGui.SetWindowSize(100, 300)
  ImGui.SetWindowSize(100, 300, ImGuiCond.Appearing)

  -- ImGui.SetWindowCollapsed(...)
  -- Parameters: bool (collapsed), ImguiCond (cond) [O]
  -- Overloads
  ImGui.SetWindowCollapsed(false)
  ImGui.SetWindowCollapsed(true, ImGuiCond.Appearing)

  -- ImGui.SetWindowFocus()
  ImGui.SetWindowFocus()

  -- ImGui.SetWindowFontScale(...)
  -- Parameters: float (scale)
  ImGui.SetWindowFontScale(1.2)

  -- ImGui.SetWindowPos(...)
  -- Parameters: text (name), float (pos_x), float (pos_y), ImGuiCond (cond) [O]
  -- Overloads
  ImGui.SetWindowPos("WindowName", 100, 100)
  ImGui.SetWindowPos("WindowName", 100, 100, ImGuiCond.Always)

  -- ImGui.SetWindowSize(...)
  -- Parameters: text (name), float (size_x), float (size_y), ImGuiCond (cond) [O]
  -- Overloads
  ImGui.SetWindowSize("WindowName", 300, 400)
  ImGui.SetWindowSize("WindowName", 300, 400, ImGuiCond.Always)

  -- ImGui.SetWindowCollapsed(...)
  -- Parameters: text (name), bool (collapsed), ImGuiCond (cond) [O]
  -- Overloads
  ImGui.SetWindowCollapsed("WindowName", true)
  ImGui.SetWindowCollapsed("WindowName", false, ImGuiCond.Always)

  -- ImGui.SetWindowFocus(...)
  -- Parameters: text (name)
  ImGui.SetWindowFocus("WindowName")
```

## Content Region
```lua
  -- ImGui.GetContentRegionMax()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetContentRegionMax()

  -- ImGui.GetContentRegionAvail()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetContentRegionAvail()

  -- ImGui.GetWindowContentRegionMin()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetWindowContentRegionMin()

  -- ImGui.GetWindowContentRegionMax()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetWindowContentRegionMax()

  -- ImGui.GetWindowContentRegionWidth()
  -- Returns: float (width)
  width = ImGui.GetWindowContentRegionWidth()
```

## Windows Scrolling
```lua
  -- ImGui.GetScrollX()
  -- Returns: float (x)
  x = ImGui.GetScrollX()

  -- ImGui.GetScrollY()
  -- Returns: float (y)
  y = ImGui.GetScrollY()

  -- ImGui.GetScrollMaxX()
  -- Returns: float (x)
  x = ImGui.GetScrollMaxX()

  -- ImGui.GetScrollMaxY()
  -- Returns: float (y)
  y = ImGui.GetScrollMaxY()

  -- ImGui.SetScrollX(...)
  -- Parameters: float (scroll_x)
  ImGui.SetScrollX(0.7)

  -- ImGui.SetScrollY(...)
  -- Parameters: float (scroll_y)
  ImGui.SetScrollY(0.7)

  -- ImGui.SetScrollHereX(...)
  -- Parameters: float (center_x_ratio) [O]
  -- Overloads
  ImGui.SetScrollHereX()
  ImGui.SetScrollHereX(0.5)

  -- ImGui.SetScrollHereY(...)
  -- Parameters: float (center_y_ratio) [O]
  -- Overloads
  ImGui.SetScrollHereY()
  ImGui.SetScrollHereY(0.5)

  -- ImGui.SetScrollFromPosX(...)
  -- Parameters: float (local_x), float (center_x_ratio) [O]
  -- Overloads
  ImGui.SetScrollFromPosX(10)
  ImGui.SetScrollFromPosX(10, 0.5)

  -- ImGui.SetScrollFromPosY(...)
  -- Parameters: float (local_y), float (center_y_ratio) [O]
  -- Overloads
  ImGui.SetScrollFromPosY(10)
  ImGui.SetScrollFromPosY(10, 0.5)
```

## Parameters Stacks (Shared)
```lua
  -- ImGui.PushStyleColor(...)
  -- Parameters A: ImGuiCol (idx), int (color_u32)
  -- Parameters B: ImGuiCol (idx), float (color_r), float (color_g), float (color_b), float (color_a)
  -- Overloads
  ImGui.PushStyleColor(ImGuiCol.Tab, 0xF42069FF)
  ImGui.PushStyleColor(ImGuiCol.Border, 1, 0, 0, 1)

  -- ImGui.PopStyleColor(...)
  -- Parameters: int (count) [O]
  -- Overloads
  ImGui.PopStyleColor()
  ImGui.PopStyleColor(5)

  -- ImGui.PushStyleVar(...)
  -- Parameters A: ImGuiStyleVar (idx), float (value)
  -- Parameters B: ImGuiStyleVar (idx), float (value_x), float (value_y)
  -- Overloads
  ImGui.PushStyleVar(ImGuiStyleVar.Alpha, 0.5)
  ImGui.PushStyleVar(ImGuiStyleVar.WindowPadding, 0.2, 0.1)

  -- ImGui.PopStyleVar(...)
  -- Parameters: int (count) [O]
  ImGui.PopStyleVar()
  ImGui.PopStyleVar(2)

  -- ImGui.GetStyleColorVec4(...)
  -- Parameters: ImGuiCol (idx)
  -- Returns: float (color_r), float (color_g), float (color_b), float (color_a)
  color_r, color_g, color_b, color_a = ImGui.GetStyleColorVec4(ImGuiCol.Text)

  -- ImGui.GetFontSize()
  -- Returns: float (fontSize)
  fontSize = ImGui.GetFontSize()

  -- ImGui.GetFontTexUvWhitePixel()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetFontTexUvWhitePixel()

  -- ImGui.GetColorU32(...)
  -- Parameters A: ImGuiCol (idx), float (alphaMultiplier, usually stays at 1)
  -- Parameters B: float (color_r), float (color_g), float (color_b), float (color_a)
  -- Returns: int (color_u32)
  -- Overloads
  color_u32 = ImGui.GetColorU32(ImGuiCol.Text, 1)
  color_u32 = ImGui.GetColorU32(0, 1, 0, 1)

```

## Parameter Stacks (Current Window)
```lua
  -- ImGui.PushItemWidth(...)
  -- Parameters: float (width)
  ImGui.PushItemWidth(100)

  -- ImGui.PopItemWidth()
  ImGui.PopItemWidth()

  -- ImGui.SetNextItemWidth(...)
  -- Parameters: float (width)
  ImGui.SetNextItemWidth(100)

  -- ImGui.CalcItemWidth()
  -- Returns: float (width)
  width = ImGui.CalcItemWidth()

  -- ImGui.PushTextWrapPos(...)
  -- Parameters: float (wrap_local_pos_x) [O]
  -- Overloads
  ImGui.PushTextWrapPos()
  ImGui.PushTextWrapPos(50)

  -- ImGui.PopTextWrapPos()
  ImGui.PopTextWrapPos()

  -- ImGui.PushAllowKeyboardFocus(...)
  -- Parameters: bool (allow_keyboard_focus)
  ImGui.PushAllowKeyboardFocus(true)

  -- ImGui.PopAllowKeyboardFocus()
  ImGui.PopAllowKeyboardFocus()

  -- ImGui.PushButtonRepeat(...)
  -- Parameters: bool (repeat)
  ImGui.PushButtonRepeat(true)

  -- ImGui.PopButtonRepeat()
  ImGui.PopButtonRepeat()
```

## Cursor / Layout
```lua
  -- ImGui.Separator()
  ImGui.Separator

  -- ImGui.SameLine(...)
  -- Parameters: float (offset_from_start_x) [O], float (spacing) [O]
  -- Overloads
  ImGui.SameLine()
  ImGui.SameLine(100)
  ImGui.SameLine(100, 5)

  -- ImGui.NewLine()
  ImGui.NewLine()

  -- ImGui.Spacing()
  ImGui.Spacing()

  -- ImGui.Dummy(...)
  -- Parameters: float (size_x), float (size_y)
  ImGui.Dummy(100, 200)

  -- ImGui.Indent(...)
  -- Parameters: float (indent_w) [O]
  ImGui.Indent()
  ImGui.Indent(10)

  -- ImGui.Unindent(...)
  -- Parameters: float (indent_w) [O]
  ImGui.Unindent()
  ImGui.Unindent(-10)

  -- ImGui.BeginGroup()
  ImGui.BeginGroup()

  -- ImGui.EndGroup()
  ImGui.EndGroup()

  -- ImGui.GetCursorPos()
  -- Returns: float (x), float(y)
  x, y = ImGui.GetCursorPos()

  -- ImGui.GetCursorPosX()
  -- Returns: float (x)
  x = ImGui.GetCursorPosX()

  -- ImGui.GetCursorPosY()
  -- Returns: float (y)
  y = ImGui.GetCursorPosY()

  -- ImGui.SetCursorPos(...)
  -- Parameters: float (x), float (y)
  ImGui.SetCursorPos(10, 10)

  -- ImGui.SetCursorPosX(...)
  -- Parameters: float (x)
  ImGui.SetCursorPosX(10)

  -- ImGui.SetCursorPosY(...)
  -- Parameters: float (y)
  ImGui.SetCursorPosY(10)

  -- ImGui.GetCursorStartPos()
  -- Returns: float (x), float(y)
  x, y = ImGui.GetCursorStartPos()

  -- ImGui.GetCursorScreenPos()
  -- Returns: float (x), float(y)
  x, y = ImGui.GetCursorScreenPos()

  -- ImGui.SetCursorScreenPos(...)
  -- Parameters: float (x), float (y)
  ImGui.SetCursorScreenPos(10, 10)

  -- ImGui.AlignTextToFramePadding()
  ImGui.AlignTextToFramePadding()

  -- ImGui.GetTextLineHeight()
  -- Returns: float (height)
  height = ImGui.GetTextLineHeight()

  -- ImGui.GetTextLineHeightWithSpacing()
  -- Returns: float (height)
  height = ImGui.GetTextLineHeightWithSpacing()

  -- ImGui.GetFrameHeight()
  -- Returns: float (height)
  height = ImGui.GetFrameHeight()

  -- ImGui.GetFrameHeightWithSpacing()
  -- Returns: float (height)
  height = ImGui.GetFrameHeightWithSpacing()
```

## ID Stack / Scopes
```lua
  -- ImGui.PushID(...)
  -- Parameters A: text (str_id)
  -- Parameters B: int (int_id)
  -- Overloads
  ImGui.PushID("MyID")
  ImGui.PushID(1)

  -- ImGui.PopID()
  ImGui.PopID()

  -- ImGui.GetID(...)
  -- Parameters A: text (str_id)
  -- Returns: int (id)
  -- Overloads
  id = ImGui.GetID("MyID")
```

## Widgets: Text
```lua
  -- ImGui.TextUnformatted(...)
  -- Parameters: text (text)
  -- Overloads
  ImGui.TextUnformatted("I am Unformatted")

  -- ImGui.Text(...)
  -- Parameters: text (text)
  ImGui.Text("Well hello there, General Kenobi")

  -- ImGui.TextColored(...)
  -- Parameters: float (color_r), float (color_g), float (color_b), float (color_a), text (text)
  ImGui.TextColored(1, 0, 0, 1, "Well hello there, General Kenobi")

  -- ImGui.TextDisabled(...)
  -- Parameters: text (text)
  ImGui.TextDisabled("Well hello there, General Kenobi")

  -- ImGui.TextWrapped(...)
  -- Parameters: text (text)
  ImGui.TextWrapped("Well hello there, General Kenobi")

  -- ImGui.LabelText(...)
  -- Parameters: text (label), text (text)
  ImGui.LabelText("Well hello there", "General Kenobi")

  -- ImGui.BulletText(...)
  -- Parameters: text (text)
  ImGui.BulletText("Well hello there, General Kenobi")
```

## Widgets: Main
```lua
  -- ImGui.Button(...)
  -- Parameters: text (label), float (size_x) [O], float (size_y) [O]
  -- Returns: bool (clicked)
  -- Overloads
  clicked = ImGui.Button("Label")
  clicked = ImGui.Button("Label", 100, 50)

  -- ImGui.SmallButton(...)
  -- Parameters: text (label)
  -- Returns: bool (clicked)
  clicked = ImGui.SmallButton("Label")

  -- ImGui.InvisibleButton(...)
  -- Parameters: text (label), float (size_x), float (size_y)
  -- Returns: bool (clicked)
  clicked = ImGui.InvisibleButton("Label", 100, 50)

  -- ImGui.ArrowButton(...)
  -- Parameters: text (str_id), ImGuiDir (dir)
  -- Returns: bool (clicked)
  clicked = ImGui.ArrowButton("I have an arrow", ImGuiDir.Down)

  -- ImGui.Checkbox(...)
  -- Parameters: text (label), bool (value)
  -- Returns: bool (value), bool (pressed)
  value, pressed = ImGui.Checkbox("My Checkbox", value)

  -- ImGui.RadioButton(...)
  -- Parameters A: text (label), bool (active)
  -- Parameters B: text (label), int (value), int (v_button)
  -- Returns A: bool (pressed)
  -- Returns B: int (value), bool (pressed)
  -- Overloads
  pressed = ImGui.RadioButton("Click me", pressed == true)
  value, pressed = ImGui.RadioButton("Click me too", value, 2)

  -- ImGui.ProgressBar(...)
  -- Parameters: float (fraction), float (size_x) [O], float (size_y) [O], text (overlay) [O]
  -- Overloads
  ImGui.ProgressBar(0.5)
  ImGui.ProgressBar(0.5, 100, 25)
  ImGui.ProgressBar(0.5, 100, 25, "Loading Failed. Sike. - 50%")

  -- ImGui.Bullet()
  ImGui.Bullet()
```

## Widgets: Combo Box
```lua
  -- ImGui.BeginCombo(...)
  -- Parameters: text (label), text (previewValue), ImGuiComboFlags (flags) [O]
  -- Returns: bool (shouldDraw)
  -- Overloads
  shouldDraw = ImGui.BeginCombo("My Combo", "Preview")
  shouldDraw = ImGui.BeginCombo("My Combo", "Preview", ImGuiComboFlags.PopupAlignLeft)

  -- ImGui.EndCombo()
  ImGui.EndCombo()

  -- ImGui.Combo(...)
  -- Parameters A: text (label), int (current_item), table (items), int (items_count), int (popup_max_height_in_items) [O]
  -- Parameters B: text (label), int (current_item), text (items_separated_by_zeros), int (popup_max_height_in_items) [O]
  -- Returns: int (current_item), bool (clicked)
  -- Overloads
  current_item, clicked = ImGui.Combo("Label", current_item, { "Option 1 ", "Option 2" }, 2)
  current_item, clicked = ImGui.Combo("Label", current_item, { "Option 1 ", "Option 2" }, 2, 5)
  current_item, clicked = ImGui.Combo("Label", current_item, "Option1\0Option2\0")
  current_item, clicked = ImGui.Combo("Label", current_item, "Option1\0Option2\0", 5)
```

## Widgets: Drags
```lua
  -- ImGui.DragFloat(...)
  -- Parameters: text (label), float (value), float (value_speed) [O], float (value_min) [O], float (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: float (value), bool (used)
  -- Overloads
  value, used = ImGui.DragFloat("Label", value)
  value, used = ImGui.DragFloat("Label", value, 0.01)
  value, used = ImGui.DragFloat("Label", value, 0.01, -10)
  value, used = ImGui.DragFloat("Label", value, 0.01, -10, 10)
  value, used = ImGui.DragFloat("Label", value, 0.01, -10, 10, "%.1f")
  value, used = ImGui.DragFloat("Label", value, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragFloat2(...)
  -- Parameters: text (label), table (values), float (value_speed) [O], float (value_min) [O], float (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.DragFloat2("Label", values)
  values, used = ImGui.DragFloat2("Label", values, 0.01)
  values, used = ImGui.DragFloat2("Label", values, 0.01, -10)
  values, used = ImGui.DragFloat2("Label", values, 0.01, -10, 10)
  values, used = ImGui.DragFloat2("Label", values, 0.01, -10, 10, "%.1f")
  values, used = ImGui.DragFloat2("Label", values, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragFloat3(...)
  -- Parameters: text (label), table (values), float (value_speed) [O], float (value_min) [O], float (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.DragFloat3("Label", values)
  values, used = ImGui.DragFloat3("Label", values, 0.01)
  values, used = ImGui.DragFloat3("Label", values, 0.01, -10)
  values, used = ImGui.DragFloat3("Label", values, 0.01, -10, 10)
  values, used = ImGui.DragFloat3("Label", values, 0.01, -10, 10, "%.1f")
  values, used = ImGui.DragFloat3("Label", values, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragFloat4(...)
  -- Parameters: text (label), table (values), float (value_speed) [O], float (value_min) [O], float (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.DragFloat4("Label", values)
  values, used = ImGui.DragFloat4("Label", values, 0.01)
  values, used = ImGui.DragFloat4("Label", values, 0.01, -10)
  values, used = ImGui.DragFloat4("Label", values, 0.01, -10, 10)
  values, used = ImGui.DragFloat4("Label", values, 0.01, -10, 10, "%.1f")
  values, used = ImGui.DragFloat4("Label", values, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragInt(...)
  -- Parameters: text (label), int (value), float (value_speed) [O], int (value_min) [O], int (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: int (value), bool (used)
  -- Overloads
  value, used = ImGui.DragInt("Label", value)
  value, used = ImGui.DragInt("Label", value, 0.01)
  value, used = ImGui.DragInt("Label", value, 0.01, -10)
  value, used = ImGui.DragInt("Label", value, 0.01, -10, 10)
  value, used = ImGui.DragInt("Label", value, 0.01, -10, 10, "%d")
  value, used = ImGui.DragInt("Label", value, 0.01, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragInt2(...)
  -- Parameters: text (label), table (values), float (value_speed) [O], int (value_min) [O], int (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.DragInt2("Label", values)
  values, used = ImGui.DragInt2("Label", values, 0.01)
  values, used = ImGui.DragInt2("Label", values, 0.01, -10)
  values, used = ImGui.DragInt2("Label", values, 0.01, -10, 10)
  values, used = ImGui.DragInt2("Label", values, 0.01, -10, 10, "%d")
  values, used = ImGui.DragInt2("Label", values, 0.01, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragInt3(...)
  -- Parameters: text (label), table (values), float (value_speed) [O], int (value_min) [O], int (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.DragInt3("Label", values)
  values, used = ImGui.DragInt3("Label", values, 0.01)
  values, used = ImGui.DragInt3("Label", values, 0.01, -10)
  values, used = ImGui.DragInt3("Label", values, 0.01, -10, 10)
  values, used = ImGui.DragInt3("Label", values, 0.01, -10, 10, "%d")
  values, used = ImGui.DragInt3("Label", values, 0.01, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.DragInt4(...)
  -- Parameters: text (label), table (values), float (value_speed) [O], int (value_min) [O], int (value_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.DragInt4("Label", values)
  values, used = ImGui.DragInt4("Label", values, 0.01)
  values, used = ImGui.DragInt4("Label", values, 0.01, -10)
  values, used = ImGui.DragInt4("Label", values, 0.01, -10, 10)
  values, used = ImGui.DragInt4("Label", values, 0.01, -10, 10, "%d")
  values, used = ImGui.DragInt4("Label", values, 0.01, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)
```

## Widgets: Sliders
```lua
  -- ImGui.SliderFloat(...)
  -- Parameters: text (label), float (value), float (value_min), float (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: float (value), bool (used)
  -- Overloads
  value, used = ImGui.SliderFloat("Label", value, -10, 10)
  value, used = ImGui.SliderFloat("Label", value, -10, 10, "%.1f")
  value, used = ImGui.SliderFloat("Label", value, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderFloat2(...)
  -- Parameters: text (label), table (values), float (value_min), float (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.SliderFloat2("Label", values, 0.01, -10, 10)
  values, used = ImGui.SliderFloat2("Label", values, 0.01, -10, 10, "%.1f")
  values, used = ImGui.SliderFloat2("Label", values, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderFloat3(...)
  -- Parameters: text (label), table (values), float (value_min), float (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.SliderFloat3("Label", values, 0.01, -10, 10)
  values, used = ImGui.SliderFloat3("Label", values, 0.01, -10, 10, "%.1f")
  values, used = ImGui.SliderFloat3("Label", values, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderFloat4(...)
  -- Parameters: text (label), table (values), float (value_min), float (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.SliderFloat4("Label", values, 0.01, -10, 10)
  values, used = ImGui.SliderFloat4("Label", values, 0.01, -10, 10, "%.1f")
  values, used = ImGui.SliderFloat4("Label", values, 0.01, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderAngle(...)
  -- Parameters: text (label), float (v_rad), float (v_degrees_min) [O], float (v_degrees_max) [O], text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: float (v_rad), bool (used)
  -- Overloads
  v_rad, used = ImGui.SliderAngle("Label", v_rad)
  v_rad, used = ImGui.SliderAngle("Label", v_rad, -255)
  v_rad, used = ImGui.SliderAngle("Label", v_rad, -255, 360)
  v_rad, used = ImGui.SliderAngle("Label", v_rad, -255, 360, "%.0f deg")
  v_rad, used = ImGui.SliderAngle("Label", v_rad, -255, 360, "%.0f deg", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderInt(...)
  -- Parameters: text (label), int (value), int (value_min), int (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: int (value), bool (used)
  -- Overloads
  value, used = ImGui.SliderInt("Label", value, -10, 10)
  value, used = ImGui.SliderInt("Label", value, -10, 10, "%d")
  value, used = ImGui.SliderInt("Label", value, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderInt2(...)
  -- Parameters: text (label), table (values), int (value_min), int (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.SliderInt2("Label", values, -10, 10)
  values, used = ImGui.SliderInt2("Label", values, -10, 10, "%d")
  values, used = ImGui.SliderInt2("Label", values, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderInt3(...)
  -- Parameters: text (label), table (values), int (value_min), int (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.SliderInt3("Label", values, -10, 10)
  values, used = ImGui.SliderInt3("Label", values, -10, 10, "%d")
  values, used = ImGui.SliderInt3("Label", values, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.SliderInt4(...)
  -- Parameters: text (label), table (values), int (value_min), int (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.SliderInt4("Label", values, -10, 10)
  values, used = ImGui.SliderInt4("Label", values, -10, 10, "%d")
  values, used = ImGui.SliderInt4("Label", values, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)

  -- ImGui.VSliderFloat(...)
  -- Parameters: text (label), float (size_x), float (size_y), float (value), float (value_min), float (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: float (value), bool (used)
  -- Overloads
  value, used = ImGui.VSliderFloat("Label", 100, 25, value, -10, 10)
  value, used = ImGui.VSliderFloat("Label", 100, 25, value, -10, 10, "%.1f")
  value, used = ImGui.VSliderFloat("Label", 100, 25, value, -10, 10, "%.1f", ImGuiSliderFlags.Logarithmic)

  -- ImGui.VSliderInt(...)
  -- Parameters: text (label), float (size_x), float (size_y), int (value), int (value_min), int (value_max), text (format) [O], ImGuiSliderFlags (flags) [O]
  -- Returns: int (value), bool (used)
  -- Overloads
  value, used = ImGui.VSliderInt("Label", 100, 25, value, -10, 10)
  value, used = ImGui.VSliderInt("Label", 100, 25, value, -10, 10, "%d")
  value, used = ImGui.VSliderInt("Label", 100, 25, value, -10, 10, "%d", ImGuiSliderFlags.Logarithmic)
```

## Widgets: Input with Keyboard
```lua
  -- ImGui.InputText(...)
  -- Parameters: text (label), text (text), int (buf_size), ImGuiInputTextFlags (flags) [O]
  -- Returns: text (text), bool (selected)
  -- Overloads
  text, selected = ImGui.InputText("Label", text, 100)
  text, selected = ImGui.InputText("Label", text, 100, ImGuiInputTextFlags.ReadOnly)

  -- ImGui.InputTextMultiline(...)
  -- Parameters: text (label), text (text), int (buf_size), float (size_x) [O], float (size_y) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: text (text), bool (selected)
  -- Overloads
  text, selected = ImGui.InputTextMultiline("Label", text, 100)
  text, selected = ImGui.InputTextMultiline("Label", text, 100, 200, 35)
  text, selected = ImGui.InputTextMultiline("Label", text, 100, 200, 35, ImGuiInputTextFlags.ReadOnly)

  -- ImGui.InputTextWithHint(...)
  -- Parameters: text (label), text (hint), text (text), int (buf_size), ImGuiInputTextFlags (flags) [O]
  -- Returns: text (text), bool (selected)
  -- Overloads
  text, selected = ImGui.InputTextWithHint("Label", "Hint", text, 100)
  text, selected = ImGui.InputTextWithHint("Label", "Hint", text, 100, ImGuiInputTextFlags.ReadOnly)

  -- ImGui.InputFloat(...)
  -- Parameters: text (label), float (value), float (step) [O], float (step_fast) [O], text (format) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: float (value), bool (used)
  -- Overloads
  value, used = ImGui.InputFloat("Label", value)
  value, used = ImGui.InputFloat("Label", value, 1)
  value, used = ImGui.InputFloat("Label", value, 1, 10)
  value, used = ImGui.InputFloat("Label", value, 1, 10, "%.1f")
  value, used = ImGui.InputFloat("Label", value, 1, 10, "%.1f", ImGuiInputTextFlags.None)

  -- ImGui.InputFloat2(...)
  -- Parameters: text (label), table (values), text (format) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.InputFloat2("Label", values)
  values, used = ImGui.InputFloat2("Label", values, "%.1f")
  values, used = ImGui.InputFloat2("Label", values, "%.1f", ImGuiInputTextFlags.None)

  -- ImGui.InputFloat3(...)
  -- Parameters: text (label), table (values), text (format) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.InputFloat3("Label", values)
  values, used = ImGui.InputFloat3("Label", values, "%.1f")
  values, used = ImGui.InputFloat3("Label", values, "%.1f", ImGuiInputTextFlags.None)

  -- ImGui.InputFloat4(...)
  -- Parameters: text (label), table (values), text (format) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.InputFloat4("Label", values)
  values, used = ImGui.InputFloat4("Label", values, "%.1f")
  values, used = ImGui.InputFloat4("Label", values, "%.1f", ImGuiInputTextFlags.None)

  -- ImGui.InputInt(...)
  -- Parameters: text (label), int (value), int (step) [O], int (step_fast) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: int (value), bool (used)
  -- Overloads
  value, used = ImGui.InputInt("Label", value)
  value, used = ImGui.InputInt("Label", value, 1)
  value, used = ImGui.InputInt("Label", value, 1, 10)
  value, used = ImGui.InputInt("Label", value, 1, 10, ImGuiInputTextFlags.None)

  -- ImGui.InputInt2(...)
  -- Parameters: text (label), table (values), ImGuiInputTextFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.InputInt2("Label", values)
  values, used = ImGui.InputInt2("Label", values, ImGuiInputTextFlags.None)

  -- ImGui.InputInt3(...)
  -- Parameters: text (label), table (values), ImGuiInputTextFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.InputInt3("Label", values)
  values, used = ImGui.InputInt3("Label", values, ImGuiInputTextFlags.None)

  -- ImGui.InputInt4(...)
  -- Parameters: text (label), table (values), ImGuiInputTextFlags (flags) [O]
  -- Returns: table (values), bool (used)
  -- Overloads
  values, used = ImGui.InputInt4("Label", values)
  values, used = ImGui.InputInt4("Label", values, ImGuiInputTextFlags.None)

  -- ImGui.InputDouble(...)
  -- Parameters: text (label), double (value), double (step) [O], double (step_fast) [O], text (format) [O], ImGuiInputTextFlags (flags) [O]
  -- Returns: double (value), bool (used)
  -- Overloads
  value, used = ImGui.InputDouble("Label", value)
  value, used = ImGui.InputDouble("Label", value, 1)
  value, used = ImGui.InputDouble("Label", value, 1, 10)
  value, used = ImGui.InputDouble("Label", value, 1, 10, "%.4f")
  value, used = ImGui.InputDouble("Label", value, 1, 10, "%.4f", ImGuiInputTextFlags.None)
```

## Widgets: Color Editor / Picker
```lua
  -- ImGui.ColorEdit3(...)
  -- Parameters: text (label), table (col), ImGuiColorEditFlags (flags) [O]
  -- Returns: table (col), bool (used)
  -- Overloads
  col, used = ImGui.ColorEdit3("Label", col)
  col, used = ImGui.ColorEdit3("Label", col, ImGuiColorEditFlags.NoTooltip)

  -- ImGui.ColorEdit4(...)
  -- Parameters: text (label), table (col), ImGuiColorEditFlags (flags) [O]
  -- Returns: table (col), bool (used)
  -- Overloads
  col, used = ImGui.ColorEdit4("Label", col)
  col, used = ImGui.ColorEdit4("Label", col, ImGuiColorEditFlags.NoTooltip)

  -- ImGui.ColorPicker3(...)
  -- Parameters: text (label), table (col), ImGuiColorEditFlags (flags) [O]
  -- Returns: table (col), bool (used)
  -- Overloads
  col, used = ImGui.ColorPicker3("Label", col)
  col, used = ImGui.ColorPicker3("Label", col, ImGuiColorEditFlags.NoTooltip)

  -- ImGui.ColorPicker4(...)
  -- Parameters: text (label), table (col), ImGuiColorEditFlags (flags) [O]
  -- Returns: table (col), bool (used)
  -- Overloads
  col, used = ImGui.ColorPicker4("Label", col)
  col, used = ImGui.ColorPicker4("Label", col, ImGuiColorEditFlags.NoTooltip)

  -- ImGui.ColorButton(...)
  -- Parameters: text (desc_id), table (col), ImGuiColorEditFlags (flags) [O], float (size_x) [O], float (size_y) [O]
  -- Returns: bool (pressed)
  -- Overloads
  pressed = ImGui.ColorButton("Desc ID", { 1, 0, 0, 1 })
  pressed = ImGui.ColorButton("Desc ID", { 1, 0, 0, 1 }, ImGuiColorEditFlags.None)
  pressed = ImGui.ColorButton("Desc ID", { 1, 0, 0, 1 }, ImGuiColorEditFlags.None, 100, 100)

  -- ImGui.SetColorEditOptions(...)
  -- Parameters: ImGuiColorEditFlags (flags)
  ImGui.SetColorEditOptions(ImGuiColorEditFlags.NoTooltip | ImGuiColorEditFlags.NoInputs)
```

## Widgets: Trees
```lua
  -- ImGui.TreeNode(...)
  -- Parameters: text (label), text (fmt) [O]
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.TreeNode("Label")
  open = ImGui.TreeNode("Label", "Some Text")

  -- ImGui.TreeNodeEx(...)
  -- Parameters: text (label), ImGuiTreeNodeFlags (flags) [O], text (fmt) [O]
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.TreeNodeEx("Label")
  open = ImGui.TreeNodeEx("Label", ImGuiTreeNodeFlags.Selected)
  open = ImGui.TreeNodeEx("Label", ImGuiTreeNodeFlags.Selected, "Some Text")

  -- ImGui.TreePush(...)
  -- Parameters: text (str_id)
  ImGui.TreePush("String ID")

  -- ImGui.TreePop()
  ImGui.TreePop()

  -- ImGui.GetTreeNodeToLabelSpacing()
  -- Returns: float (spacing)
  spacing = ImGui.GetTreeNodeToLabelSpacing()

  -- ImGui.CollapsingHeader(...)
  -- Parameters A: text (label), ImGuiTreeNodeFlags (flags) [O]
  -- Parameters B: text (label), bool (open), ImGuiTreeNodeFlags (flags) [O]
  -- Returns A: bool (notCollapsed)
  -- Returns B: bool (open), bool (notCollapsed)
  -- Overloads
  notCollapsed = ImGui.CollapsingHeader("Label")
  notCollapsed = ImGui.CollapsingHeader("Label", ImGuiTreeNodeFlags.Selected)
  open, notCollapsed = ImGui.CollapsingHeader("Label", open)
  open, notCollapsed = ImGui.CollapsingHeader("Label", open, ImGuiTreeNodeFlags.Selected)

  -- ImGui.SetNextItemOpen(...)
  -- Parameters: bool (open), ImGuiCond (cond) [O]
  -- Overloads
  ImGui.SetNextItemOpen(true)
  ImGui.SetNextItemOpen(true, ImGuiCond.Always)
```

## Widgets: Selectables
```lua
  -- ImGui.Selectable(...)
  -- Parameters: text (label), bool (selected) [O], ImGuiSelectableFlags (flags) [O], float (size_x) [O], float (size_y) [O]
  -- Returns: bool (selected)
  -- Overloads
  selected = ImGui.Selectable("Label")
  selected = ImGui.Selectable("Label", selected)
  selected = ImGui.Selectable("Label", selected, ImGuiSelectableFlags.AllowDoubleClick)
  selected = ImGui.Selectable("Label", selected, ImGuiSelectableFlags.AllowDoubleClick, 100, 100)
```

## Widgets: List Boxes
```lua
  -- ImGui.ListBox(...)
  -- Parameters: text (label), int (current_item), table (items), int (items_count), int (height_in_items) [O]
  -- Returns: int (current_item), bool (clicked)
  -- Overloads
  current_item, clicked = ImGui.ListBox("Label", current_item, { "Item 1", "Item 2", 2 })
  current_item, clicked = ImGui.ListBox("Label", current_item, { "Item 1", "Item 2", 2 }, 5)

  -- ImGui.BeginListBox(...)
  -- Parameters A: text (label), float (size_x) [O], float (size_y) [O]
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginListBox("Label")
  open = ImGui.BeginListBox("Label", 100.0, 100.0)

  -- ImGui.EndListBox()
  ImGui.EndListBox()
```

## Widgets: Value() Helpers
```lua
  -- ImGui.Value(...)
  -- Parameters: text (prefix) bool/int/unsigned int/float (value), text (float_format) [O] -- format only available with float
  -- Overloads
  ImGui.Value("Prefix", true)
  ImGui.Value("Prefix", -5)
  ImGui.Value("Prefix", 5)
  ImGui.Value("Prefix", 5.0)
  ImGui.Value("Prefix", 5.0, "%.2f")
```

## Widgets: Menus
```lua
-- ImGui.BeginMenuBar()
-- Returns: bool (shouldDraw)
shouldDraw = ImGui.BeginMenuBar()

-- ImGui.EndMenuBar()
ImGui.EndMenuBar()

-- ImGui.BeginMainMenuBar()
-- Returns: bool (shouldDraw)
shouldDraw = ImGui.BeginMainMenuBar()

-- ImGui.EndMainMenuBar()
ImGui.EndMainMenuBar()

-- ImGui.BeginMenu(...)
-- Parameters: text (label), bool (enabled) [O]
-- Returns: bool (shouldDraw)
-- Overloads
shouldDraw = ImGui.BeginMenu("Label")
shouldDraw = ImGui.BeginMenu("Label", true)

-- ImGui.EndMenu()
ImGui.EndMenu()

-- ImGui.MenuItem(...)
-- Parameters A: text (label), text (shortcut) [0]
-- Parameters B: text (label), text (shortcut), bool (selected)
-- Returns A: bool (activated)
-- returns B: bool (selected), bool (activated)
-- Overloads
activated = ImGui.MenuItem("Label")
activated = ImGui.MenuItem("Label", "ALT+F4")
selected, activated = ImGui.MenuItem("Label", "ALT+F4", selected)
selected, activated = ImGui.MenuItem("Label", "ALT+F4", selected, true)
```

## Tooltips
```lua
  -- ImGui.BeginTooltip()
  ImGui.BeginTooltip()

  -- ImGui.EndTooltip()
  ImGui.EndTooltip()

  -- ImGui.SetTooltip(...)
  -- Parameters: text (fmt)
  ImGui.SetTooltip("Did you know that I have the high ground?")
```

## Popups, Modals
```lua
  -- ImGui.BeginPopup(...)
  -- Parameters: text (str_id), ImGuiWindowFlags (flags) [O]
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginPopup("String ID")
  open = ImGui.BeginPopup("String ID", ImGuiWindowFlags.NoCollapse)

  -- ImGui.BeginPopupModal(...)
  -- Parameters: text (name), bool (open) [O], ImGuiWindowFlags (flags) [O]
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginPopupModal("Name")
  open = ImGui.BeginPopupModal("Name", ImGuiWindowFlags.NoCollapse)
  open = ImGui.BeginPopupModal("Name", open)
  open = ImGui.BeginPopupModal("Name", open, ImGuiWindowFlags.NoCollapse)

  -- ImGui.EndPopup()
  ImGui.EndPopup()

  -- ImGui.OpenPopup(...)
  -- Parameters: text (str_id), ImGuiPopupFlags (popup_flags)
  -- Overloads
  ImGui.OpenPopup("String ID")
  ImGui.OpenPopup("String ID", ImGuiPopupFlags.NoOpenOverExistingPopup)

  -- ImGui.OpenPopupContextItem(...)
  -- Parameters: text (str_id), ImGuiPopupFlags (popup_flags)
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.OpenPopupContextItem()
  open = ImGui.OpenPopupContextItem("String ID")
  open = ImGui.OpenPopupContextItem("String ID", ImGuiPopupFlags.NoOpenOverExistingPopup)

  -- ImGui.CloseCurrentPopup()
  ImGui.CloseCurrentPopup()

  -- ImGui.BeginPopupContextItem(...)
  -- Parameters: text (str_id), ImGuiPopupFlags (popup_flags)
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginPopupContextItem()
  open = ImGui.BeginPopupContextItem("String ID")
  open = ImGui.BeginPopupContextItem("String ID", ImGuiPopupFlags.NoOpenOverExistingPopup)

  -- ImGui.BeginPopupContextWindow(...)
  -- Parameters: text (str_id), ImGuiPopupFlags (popup_flags)
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginPopupContextWindow()
  open = ImGui.BeginPopupContextWindow("String ID")
  open = ImGui.BeginPopupContextWindow("String ID", ImGuiPopupFlags.NoOpenOverExistingPopup)

  -- ImGui.BeginPopupContextVoid(...)
  -- Parameters: text (str_id), ImGuiPopupFlags (popup_flags)
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginPopupContextVoid()
  open = ImGui.BeginPopupContextVoid("String ID")
  open = ImGui.BeginPopupContextVoid("String ID", ImGuiPopupFlags.NoOpenOverExistingPopup)

  -- ImGui.IsPopupOpen(...)
  -- Parameters: text (str_id), ImGuiPopupFlags (popup_flags)
  -- Overloads
  ImGui.IsPopupOpen("String ID")
  ImGui.IsPopupOpen("String ID", ImGuiPopupFlags.NoOpenOverExistingPopup)
```

## Tables
```lua
  -- ImGui.BeginTable(...)
  -- Parameters: string (str_id), int (column), ImGuiTableFlags (flags) [O], float (outer_size_x) [O], float (outer_size_y) [O], float (inner_width) [O]
  -- Returns: bool
  ImGui.BeginTable("Table1", 3)
  ImGui.BeginTable("Table1", 3, ImGuiTableFlags.Resizable)
  ImGui.BeginTable("Table1", 3, ImGuiTableFlags.Resizable, 200, 150)
  ImGui.BeginTable("Table1", 3, ImGuiTableFlags.Resizable, 200, 150, 10)

  -- ImGui.EndTable() // only call EndTable() if BeginTable() returns true!
  ImGui.EndTable()

  -- ImGui.TableNextRow(...) // append into the first cell of a new row.
  -- Parameters: ImGuiTableRowFlags (flags) [O], float (min_row_height) [O]
  ImGui.TableNextRow()
  ImGui.TableNextRow(ImGuiTableRowFlags.Headers)
  ImGui.TableNextRow(ImGuiTableRowFlags.Headers, 25)

  -- ImGui.TableNextColumn() // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
  -- Returns: bool (visible)
  visible = ImGui.TableNextColumn()

  -- ImGui.TableSetColumnIndex(...) // append into the specified column. Return true when column is visible.
  -- Parameter: int (column_n)
  -- Returns: bool (visible)
  visible = ImGui.TableSetColumnIndex(2)

  -- ImGui.TableSetupColumn(...)
  -- Parameters: string (label), ImGuiTableColumnFlags (flags) [O], float (init_width_or_weight) [O], ImU32 (user_id) [O]
  ImGui.TableSetupColumn("Column1")
  ImGui.TableSetupColumn("Column1", ImGuiTableColumnFlags.WidthFixed)
  ImGui.TableSetupColumn("Column1", ImGuiTableColumnFlags.WidthFixed, 60)

  -- ImGui.TableSetupScrollFreeze(...) // lock columns/rows so they stay visible when scrolled.
  -- Parameters: int (cols), int(rows)
  ImGui.TableSetupScrollFreeze(3, 1)

  -- ImGuui.TableHeadersRow() // submit all headers cells based on data provided to TableSetupColumn() + submit context menu
  ImGui.TableHeadersRow()

  -- ImGui.TableHeader(...) // submit one header cell manually (rarely used)
  -- Parameter: string (label)
  ImGui.TableHeader("Header")

  -- ImGui.TableGetSortSpecs() // get latest sort specs for the table (NULL if not sorting).
  -- Returns: ImGuiTableSortSpecs*
  ImGui.TableGetSortSpecs()

  -- ImGui.TableGetColumnCount() // return number of columns (value passed to BeginTable)
  -- Returns: int (cols)
  cols = ImGui.TableGetColumnCount()

  -- ImGui.TableGetColumnIndex() // return current column index.
  -- Returns: int (col_index)
  col_index = ImGui.TableGetColumnIndex()

  -- ImGui.TableGetRowIndex() // return current row index.
  -- Returns: int (row_index)
  row_index = ImGui.TableGetRowIndex()

  -- ImGui.TableGetColumnName(...) // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
  -- Parameter: int (column_n) [O]
  -- Returns: string(col_name)
  col_name = ImGui.TableGetColumnName()
  col_name = ImGui.TableGetColumnName(2)

  -- ImGui.TableGetColumnFlags(...) // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
  -- Parameter: int (column_n) [O]
  -- Returns: ImGuiTableColumnFlags
  col_flags = ImGui.TableGetColumnFlags()
  col_flags = ImGui.TableGetColumnFlags(2)

  -- ImGui.TableSetBgColor(ImGuiTableBgTarget target, ImU32 color, int column_n = -1) // change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.
  -- Parameters1: ImGuiTableBgTarget (target), ImU32 (color), int (column_n) [O]
  -- Parameters2: ImGuiTableBgTarget (target), float (col_R), float (col_G), float (col_B), float (col_A), int (column_n) [O]
  ImGui.TableSetBgColor(ImGuiTableBgTarget.CellBg, 0xF42069FF)
  ImGui.TableSetBgColor(ImGuiTableBgTarget.CellBg, 0xF42069FF, 2)
  ImGui.TableSetBgColor(ImGuiTableBgTarget.CellBg, 1, 0, 0, 1)
  ImGui.TableSetBgColor(ImGuiTableBgTarget.CellBg, 1, 0, 0, 1, 2)
```


## Columns (Legacy API, prefer using Tables!)
```lua
  -- ImGui.Columns(...)
  -- Parameters: int (count) [O], text (id) [O], bool (border) [O]
  -- Overloads
  ImGui.Columns()
  ImGui.Columns(2)
  ImGui.Columns(2, "MyOtherColumn")
  ImGui.Columns(3, "MyColumnWithBorder", true)

  -- ImGui.NextColumn()
  ImGui.NextColumn()

  -- ImGui.GetColumnIndex()
  -- Returns: int (index)
  index = ImGui.GetColumnIndex()

  -- ImGui.GetColumnWidth(...)
  -- Parameters: int (column_index) [O]
  -- Returns: float (width)
  -- Overloads
  width = ImGui.GetColumnWidth()
  width = ImGui.getColumnWidth(2)

  -- ImGui.SetColumnWidth(...)
  -- Parameters: int (column_index), float (width)
  ImGui.SetColumnWidth(2, 100)

  -- ImGui.GetColumnOffset(...)
  -- Parameters: int (column_index) [O]
  -- Returns: float (offset)
  -- Overloads
  offset = ImGui.GetColumnOffset()
  offset = ImGui.GetColumnOffset(2)

  -- ImGui.SetColumnOffset(...)
  -- Parameters: int (column_index), float (offset)
  ImGui.SetColumnOffset(2, 10)

  -- ImGui.GetColumnsCount()
  -- Returns: int (count)
  count = ImGui.GetColumnsCount()
```

## Tab Bars, Tabs
```lua
  -- ImGui.BeginTabBar(...)
  -- Parameters: text (str_id), ImGuiTabBarFlags (flags)
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginTabBar("String ID")
  open = ImGui.BeginTabBar("String ID", ImGuiTabBarFlags.Reorderable)

  -- ImGui.EndTabBar()
  ImGui.EndTabBar()

  -- ImGui.BeginTabItem()
  -- Parameters A: text (label)
  -- Parameters B: text (label), bool (open), ImGuiTabItemFlags (flags) [O]
  -- Returns A: bool (selected)
  -- Returns B: bool (open), bool (selected)
  -- Overloads
  selected = ImGui.BeginTabItem("Label")
  selected = ImGui.BeginTabItem("Label", ImGuiTabItemFlags.NoTooltip)
  open, selected = ImGui.BeginTabItem("Label", open)
  open, selected = ImGui.BeginTabItem("Label", open, ImGuiTabItemFlags.NoTooltip)

  -- ImGui.EndTabItem()
  ImGui.EndTabItem()

  -- ImGui.SetTabItemClosed(...)
  -- Parameters: text (tab_or_docked_window_label)
  ImGui.SetTabItemClosed("MyDockedWindow")
```

## Clipping
```lua
  -- ImGui.PushClipRect(...)
  -- Parameters: float (min_x), float (min_y), float (max_x), float (max_y), bool (intersect_current)
  ImGui.PushClipRect(0, 0, 100, 100, false)

  -- ImGui.PopClipRect()
  ImGui.PopClipRect()
```

## Focus, Activation
```lua
  -- ImGui.SetItemDefaultFocus()
  ImGui.SetItemDefaultFocus()

  -- ImGui.SetKeyboardFocusHere(...)
  -- Parameters: int (offset) [O]
  -- Overloads
  ImGui.SetItemDefaultFocus()
  ImGui.SetItemDefaultFocus(5)
```

## Item / Widgets Utilities
```lua
  -- ImGui.IsItemHovered(...)
  -- Parameters: ImGuiHoveredFlags (flags) [O]
  -- Returns: bool (hovered)
  -- Overloads
  hovered = ImGui.IsItemHovered()
  hovered = ImGui.IsItemHovered(ImGuiHoveredFlags.ChildWindows)

  -- ImGui.IsItemActive()
  -- Returns: bool (active)
  active = ImGui.IsItemActive()

  -- ImGui.IsItemFocused()
  -- Returns: bool (focused)
  focused = ImGui.IsItemFocused()

  -- ImGui.IsItemClicked(...)
  -- Parameters: ImGuiMouseButton (mouse_button) [O]
  -- Returns: bool (clicked)
  -- Overloads
  clicked = ImGui.IsItemClicked()
  clicked = ImGui.IsItemClicked(ImGuiMouseButton.Middle)

  -- ImGui.IsItemVisible()
  -- Returns: bool (visible)
  visible = ImGui.IsItemVisible()

  -- ImGui.IsItemEdited()
  -- Returns: bool (edited)
  edited = ImGui.IsItemEdited()

  -- ImGui.IsItemActivated()
  -- Returns: bool (activated)
  activated = ImGui.IsItemActivated()

  -- ImGui.IsItemDeactivated()
  -- Returns: bool (deactivated)
  deactivated = ImGui.IsItemDeactivated()

  -- ImGui.IsItemDeactivatedAfterEdit()
  -- Returns: bool (deactivated_after_edit)
  deactivated_after_edit = ImGui.IsItemDeactivatedAfterEdit()

  -- ImGui.IsItemToggledOpen()
  -- Returns: bool (toggled_open)
  toggled_open = ImGui.IsItemToggledOpen()

  -- ImGui.IsAnyItemHovered()
  -- Returns: bool (any_item_hovered)
  any_item_hovered = ImGui.IsAnyItemHovered()

   -- ImGui.IsAnyItemActive()
  -- Returns: bool (any_item_active)
  any_item_active = ImGui.IsAnyItemActive()

  -- ImGui.IsAnyItemFocused()
  -- Returns: bool (any_item_focused)
  any_item_focused = ImGui.IsAnyItemFocused()

  -- ImGui.GetItemRectMin()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetItemRectMin()

  -- ImGui.GetItemRectMax()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetItemRectMax()

  -- ImGui.GetItemRectSize()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetItemRectSize()

  -- ImGui.SetItemAllowOverlap()
  ImGui.SetItemAllowOverlap()
```

## Miscellaneous Utilities
```lua
  -- ImGui.IsRectVisible(...)
  -- Parameters A: float (size_x), float (size_y)
  -- Parameters B: float(min_x), float (min_y), float (max_x), float (max_y)
  -- Returns: bool (visible)
  -- Overloads
  visible = ImGui.IsRectVisible(100, 100)
  visible = ImGui.IsRectVisible(50, 50, 200, 200)

  -- ImGui.GetTime()
  -- Returns double (time)
  time = ImGui.GetTime()

  -- ImGui.GetFrameCount()
  -- Returns int (frame_count)
  frame_count = ImGui.GetFrameCount()

  -- ImGui.GetStyleColorName(...)
  -- Parameters: ImGuiCol (idx)
  -- Returns: text (style_color_name)
  style_color_name = ImGui.GetStyleColorName(ImGuiCol.Text)

  -- ImGui.BeginChildFrame(...)
  -- Parameters: unsigned int (id), float (size_x), float (size_y), ImGuiWindowFlags (flags) [O]
  -- Returns: bool (open)
  -- Overloads
  open = ImGui.BeginChildFrame(0, 100, 100)
  open = ImGui.BeginChildFrame(0, 100, 100, ImGuiWindowFlags.NoBackground)

  -- ImGui.EndChildFrame()
  ImGui.EndChildFrame()
```

## Text Utilities
```lua
  -- ImGui.CalcTextSize(...)
  -- Parameters: text (text), bool (hide_text_after_double_hash) [O], float (wrap_width) [O]
  -- Returns: float (x), float (y)
  -- Overloads
  x, y = ImGui.CalcTextSize("Calculate me")
  x, y = ImGui.CalcTextSize("Calculate me", true)
  x, y = ImGui.CalcTextSize("Calculate me", true, 100)
```

## Color Utilities
```lua
  -- ImGui.ColorConvertRGBtoHSV(...)
  -- Parameters: float (r), float (g), float (b)
  -- Returns: float (h), float (s), float (v)
  h, s, v = ImGui.ColorConvertRGBtoHSV(1, 0, 0.5)

  -- ImGui.ColorConvertHSVtoRGB(...)
  -- Parameters: float (h), float (s), float (v)
  -- Returns: float (r), float (g), float (b)
  r, g, b = ImGui.ColorConvertHSVtoRGB(1, 0, 0.5)

  -- ImGui.ColorConvertU32ToFloat4(...)
  -- Parameters: int (color_u32)
  -- Returns: float array (color_f4={r,g,b,a})
  color_f4 = ImGui.ColorConvertU32ToFloat4(0xF69420FF)

  -- ImGui.ColorConvertFloat4ToU32(...)
  -- Parameters: float array (color_f4={r,g,b,a})
  -- Returns: int (color_u32)
  -- NOTE: this function is fundamentally
  color_u32 = ImGui.ColorConvertFloat4ToU32({0.4, 0.2, 0, 1})
```

## Inputs Utilities: Mouse
```lua
  -- ImGui.IsMouseHoveringRect(...)
  -- Parameters: float (min_x), float (min_y), float(max_x), float(max_y), bool (clip) [O]
  -- Returns: bool (hovered)
  hovered = ImGui.IsMouseHoveringRect(0, 0, 100, 100)
  hovered = ImGui.IsMouseHoveringRect(0, 0, 100, 100, true)

  -- ImGui.GetMousePos()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetMousePos()

  -- ImGui.GetMousePosOnOpeningCurrentPopup()
  -- Returns: float (x), float (y)
  x, y = ImGui.GetMousePosOnOpeningCurrentPopup()

  -- ImGui.IsMouseDragging(...)
  -- Parameters: ImGuiMouseButton (button), float (lock_threshold) [O]
  -- Returns: bool (dragging)
  -- Overloads
  dragging = ImGui.IsMouseDragging(ImGuiMouseButton.Middle)
  dragging = ImGui.IsMouseDragging(ImGuiMouseButton.Middle, 0.5)

  -- ImGui.GetMouseDragDelta(...)
  -- Parameters: ImGuiMouseButton (button) [O], float (lock_threshold) [O]
  -- Returns: float (x), float (y)
  -- Overloads
  x, y = ImGui.GetMouseDragDelta()
  x, y = ImGui.GetMouseDragDelta(ImGuiMouseButton.Middle)
  x, y = ImGui.GetMouseDragDelta(ImGuiMouseButton.Middle, 0.5)

  -- ImGui.ResetMouseDragDelta(...)
  -- Parameters: ImGuiMouseButton (button) [O]
  -- Overloads
  ImGui.ResetMouseDragDelta()
  ImGui.ResetMouseDragDelta(ImGuiMouseButton.Middle)
```

## Clipboard Utilities
```lua
  -- ImGui.GetClipboardText()
  -- Returns: text (text)
  text = ImGui.GetClipboardText()

  -- ImGui.SetClipboardText(...)
  -- Parameters: text (text)
  ImGui.SetClipboardText("I made it to the clipboard!")
```
