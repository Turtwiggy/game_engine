#include "pch.hpp"

#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_helpers.hpp"
#include "select_modifiers_components.hpp"

namespace game2d {

void
Option_Rocks::update(entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 0, 1); // true or false.

  // update value.
  populate_rocks = (bool)hindex;
}

void
Option_EnemyHealth::update(entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 1, 100); // 1x - 100x

  // update value.
  multiplier = (int)hindex;
}

void
Option_EnemyCount::update(entt::registry& r, int& hindex)
{
  hindex = glm::clamp(hindex, 1, 50); // 1x - 5x

  // update value. allow 0.1 increments
  multiplier = (hindex / 10.0f);
}

void
SINGLE_UISelectModifiersMenuState::do_init(entt::registry& r)
{
  for (int i = 0; i < (int)MODIFIER_OPTIONS::count; i++) {
    const auto enum_val = magic_enum::enum_cast<MODIFIER_OPTIONS>(i).value();
    const auto enum_str = std::string(magic_enum::enum_name(enum_val));

    OptionsCell c;
    c.name = modifier_option_enum_to_display_string(r, enum_val);
    c.value = 0;

    if (enum_val == MODIFIER_OPTIONS::ENEMY_HEALTH)
      c.value = 1;
    if (enum_val == MODIFIER_OPTIONS::ENEMY_COUNT)
      c.value = 10;

    state.cells.push_back(std::make_shared<OptionsCell>(c));
  }

  OptionsCell c;
  c.name = "Next";
  c.value = 0;
  c.action = [&r]() { move_to_scene_start(r, Scene::select_ships); };
  state.cells.push_back(std::make_shared<OptionsCell>(c));

  //
  // nav
  //
  create_as_vertical_layout(state.cells);

  state.active = state.cells[0];
  init = true;
}

} // namespace game2d