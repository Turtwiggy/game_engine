#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_quit/quit_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "ui_popup_pause_components.hpp"

namespace game2d {

void
SINGLE_PauseMenuState::do_init(entt::registry& r)
{
  const auto resume_action = [&]() { open = false; };
  const auto options_action = [&]() {
    open = false;
    create_empty<RequestToShowOptionsMenu>(r);
  };
  const auto quit_to_menu_action = [&]() {
    open = false;

    // hack: clear inputs to stop play button being clicked this frame
    auto input_e = get_first<InputComponent, Persistent>(r);
    auto& input_c = r.get<InputComponent>(input_e);
    input_c.button_s.clear();

    move_to_scene_start(r, Scene::menu);
  };
  auto quit_to_desktop_action = [&]() { create_empty<RequestQuitApplication>(r); };

  const auto make_cell = [&](auto name, auto action) {
    Cell c;
    c.name = name;
    c.action = action;
    state.cells.push_back(std::make_shared<Cell>(c));
  };

  make_cell("Resume", resume_action);
  make_cell("Options", options_action);
  make_cell("Quit To Menu", quit_to_menu_action);
  make_cell("Quit To Desktop", quit_to_desktop_action);

  //
  // nav
  //
  create_as_vertical_layout(state.cells);

  state.active = state.cells[0];
  init = true;
};

} // namespace game2d