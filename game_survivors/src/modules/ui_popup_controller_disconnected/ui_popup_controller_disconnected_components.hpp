#pragma once

#include "modules/ui_common/ui_common_components.hpp"
#include "steam/isteaminput.h"
#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_DisconnectedControllerUI
{
  std::vector<InputHandle_t> handle_disconnected;

  bool init = false;
  bool open = false;
  UIState state;
};

} // namespace game2d