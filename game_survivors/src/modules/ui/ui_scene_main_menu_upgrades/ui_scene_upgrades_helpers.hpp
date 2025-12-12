#pragma once

#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

int
get_grid_y(int n_cells, int grid_x);

void
back_to_main_menu(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c);

std::pair<int, int>
get_upgrade_level(entt::registry& r, SINGLE_PersistentUpgrades& upgrade_c, std::string stat_key);

void
process_input_for_grid(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c);

void
draw_purchasebar(entt::registry& r,
                 const ImVec2 tl,
                 const ImVec2 br,
                 const float percent,
                 const std::string text,
                 const engine::SRGBColour& col);

std::string
get_gridcell_item_key(entt::registry& r, const GridCell* gc);

int
get_item_key_cost(entt::registry& r, std::string key);

bool
hold_button_limit_reached(bool input, float& cur, const float max, const float dt);

} // namespace game2d