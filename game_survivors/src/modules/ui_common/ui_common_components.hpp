#pragma once

#include "engine/colour/colour.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace game2d {

struct SINGLE_UIData
{
  float scaling = 1.0f;
};

struct SelectableButtonDef
{
  std::string label;
  ImVec2 size{ 20, 20 };
  bool input;

  // layout index
  // note: hovering the selected button chan change the sel_index
  int my_row_index = 0;
  int my_col_index = 0;
  int& ui_row_index;
  int& ui_col_index;
  bool ui_col_active = true;

  bool text_centered = true;
  ImVec2 text_offset{ 0, 0 };
  FontSize font_size = FontSize::MENU_BUTTONS;
  FontSize font_size_scaled = FontSize::MENU_BUTTONS_SCALED;

  engine::SRGBColour active_outline_col = hex_to_srgb("#FFFFFF", 255);
  engine::SRGBColour inactive_outline_col = hex_to_srgb("#FFFFFF", 0.6f * 255);
  engine::SRGBColour active_bg_col = hex_to_srgb("#02526D", 255);
  engine::SRGBColour inactive_bg_col = hex_to_srgb("#02526D", 0.6f * 255);
};

struct RowState
{
  std::string col_name = "default";
  int col_index = 0;

  // one function per row-state?
  std::function<void()> action;
};

enum class UIAction
{
  SELECT,
  BACK,
  H_VALUE_CHANGED,
  V_VALUE_CHANGED,
  V_VALUE_CHANGED_UP,
  V_VALUE_CHANGED_DOWN,
};

// Note: this is limited.
// should support things like navigating left&right
struct UIState
{
  bool init = false;

  // vertical select
  int current_row_index = 0;
  std::vector<RowState> rows;

  std::vector<UIAction> actions;
};

} // namespace game2d