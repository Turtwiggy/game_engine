#pragma once

#include "modules/ui_common/ui_common_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_GameoverUI
{
  bool init = false;
  bool open = false;
  UIState state;
};

} // namespace game2d