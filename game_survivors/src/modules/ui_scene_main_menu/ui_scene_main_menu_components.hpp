#pragma once

#include "modules/ui_common/ui_common_components.hpp"

#include <imgui.h>

#include <string>
#include <vector>

namespace game2d {

struct SINGLE_MainMenuUI
{
  std::vector<std::string> hovered_buttons;

  // prevent immediately doing action when sent by other scene
  bool one_frame_buffer = true;

  UIState state;
};

struct MenuToNextSceneInfo
{
  int level = 0;

  bool processed = false;
};

} // namespace game2d