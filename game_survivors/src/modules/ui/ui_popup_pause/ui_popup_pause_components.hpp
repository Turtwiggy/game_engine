#pragma once

#include "modules/core/ui/ui_common_components.hpp"

namespace game2d {

struct RequestToShowPauseMenu
{
  bool placeholder = true;
};

struct SINGLE_PauseMenuState
{
  bool open = false;
  UIState state;
};

} // namespace game2d