#pragma once

#include "modules/ui_common/ui_common_components.hpp"
#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_LevelUpUI
{
  bool require_level_up = false;

  UIState state;
};

} // namespace game2d