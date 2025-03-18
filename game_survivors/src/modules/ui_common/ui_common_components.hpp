#pragma once

#include <imgui.h>

#include <functional>
#include <string>
#include <vector>

namespace game2d {

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
};

// Note: this is limited.
// should support things like navigating left&right
struct UIState
{
  bool init = false;

  // vertical select
  int current_row_index = 0;
  std::vector<RowState> rows;

  std::vector<UIAction> new_actions;
};

} // namespace game2d