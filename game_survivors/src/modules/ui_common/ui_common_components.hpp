#pragma once

#include <imgui.h>

#include <string>

namespace game2d {

struct SelectableButtonDef
{
  std::string label;
  ImVec2 size{ 20, 20 };
  int index = 0; // position in layout

  // reference to consume input
  bool& input;

  // layout index
  // note: hovering the selected button chan change the sel_index
  int& sel_index;
};

// Note: this is limited.
// should support things like navigating left&right
struct UIState
{
  int selected = 0;
  int max = 1;
  bool do_action = false;
};

} // namespace game2d