#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_quit/quit_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "ui_scene_main_menu_components.hpp"

namespace game2d {

void
SINGLE_MainMenuUI::do_init(entt::registry& r)
{
  const auto play_action = [&]() {
    open = false;
    move_to_scene_start(r, Scene::select_ships);
  };
  const auto modifiers_action = [&]() {
    open = false;
    move_to_scene_start(r, Scene::select_modifiers);
  };
  const auto upgrade_action = [&]() {
    open = false;

    // hack: clear inputs to stop play button being clicked this frame
    auto input_e = get_first<InputComponent, Persistent>(r);
    auto& input_c = r.get<InputComponent>(input_e);
    input_c.button_s.clear();

    create_empty<RequestToShowUpgradesMenu>(r);
  };
  const auto options_action = [&]() {
    open = false;
    create_empty<RequestToShowOptionsMenu>(r);
  };
  const auto exit_action = [&r]() { create_empty<RequestQuitApplication>(r); };

  const auto make_cell = [&](auto name, auto action) -> std::shared_ptr<Cell>& {
    Cell c;
    c.name = name;
    c.action = action;
    state.cells.push_back(std::make_shared<Cell>(c));
    return state.cells.back();
  };

  // #define DEBUG_SNAKE 1

#if defined(_DEBUG) && defined(DEBUG_SNAKE)
  const auto debug_snake_action = [&]() {
    open = false;
    move_to_scene_start(r, Scene::develop_snake);
  };
  auto debug_snake = make_cell("Debug Snake", debug_snake_action);
#endif

#define DEBUG_ISLANDS 0
#if defined(_DEBUG) && defined(DEBUG_ISLANDS)
  const auto debug_islands_action = [&]() { move_to_scene_start(r, Scene::develop_islands); };
  const auto debug_islands = make_cell("Debug Islands", debug_islands_action);
#endif

  auto a = make_cell("Play", play_action);
  auto b = make_cell("Upgrades", upgrade_action);
  auto c = make_cell("Options", options_action);
  auto d = make_cell("Exit", exit_action);
  auto e = make_cell("Modifiers", modifiers_action);

  //
  // nav
  //
  std::vector<std::shared_ptr<Cell>> vertical_cells = { a, b, c, d };
#if defined(_DEBUG) && defined(DEBUG_SNAKE)
  vertical_cells.push_back(debug_snake);
#endif
#if defined(_DEBUG) && defined(DEBUG_ISLANDS)
  vertical_cells.push_back(debug_islands);
#endif
  create_as_vertical_layout(vertical_cells);

  // put the modifiers button on the right of the play button
  a->r = e;
  e->l = a;

  state.active = state.cells[0];
  open = true;
  init = true;
}

} // namespace game2d