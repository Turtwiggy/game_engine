#pragma once

#include "modules/core/ui/ui_common_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_GameoverUI : public DefaultUI
{
  SINGLE_GameoverUI()
  {
    Cell c;
    c.name = "Main Menu";
    state.cells.push_back(std::make_shared<Cell>(c));

    state.active = state.cells[0];
  }
};

} // namespace game2d