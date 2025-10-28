#pragma once

#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

int
get_grid_y(int n_cells, int grid_x);

void
back_to_main_menu(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c);

void
purchase_upgrade(entt::registry& r, const UpgradeableStat stat);

std::pair<int, int>
get_upgrade_level(entt::registry& r, SINGLE_PersistentUpgrades& upgrade_c, std::string stat_key);

void
process_input_for_grid(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c);

} // namespace game2d