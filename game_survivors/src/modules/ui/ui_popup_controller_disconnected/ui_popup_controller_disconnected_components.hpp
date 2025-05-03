#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include "steam/isteaminput.h"
#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_DisconnectedControllerUI : public DefaultUI
{
  std::vector<InputHandle_t> handle_disconnected;

  SINGLE_DisconnectedControllerUI()
  {
    Cell c;
    c.name = "Resume";
    state.cells.push_back(std::make_shared<Cell>(c));

    state.active = state.cells[0];
  }
};

} // namespace game2d