#pragma once

#include "modules/ui_common/ui_common_components.hpp"

namespace game2d {

struct RequestToShowPauseMenu
{
  bool placeholder = true;
};

struct SINGLE_PauseMenuState
{
  UIState state;
};

} // namespace game2d