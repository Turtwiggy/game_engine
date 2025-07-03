#pragma once

#include "ui_back_button_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
UI_BackButton::do_init(entt::registry& r)
{
  Cell c;
  c.name = "Back (Esc)";
  c.action = [&]() { move_to_scene_start(r, Scene::menu); };
  state.cells.push_back(std::make_shared<Cell>(c));

  state.active = state.cells[0];
  init = true;
};

} // namespace game2d