#pragma once

#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
generate_upgrades_for_players(entt::registry& r, SINGLE_LevelUpUI& ui_c);

void
populate_ui_based_on_upgrades(entt::registry& r, SINGLE_LevelUpUI& ui_c);

bool
is_choosing_upgrade(entt::registry& r);

entt::entity
get_player_e_from_idx(entt::registry& r, int player_idx);

SINGLE_UpgradeToName
load_upgrade_names(const std::string& path);

} // namespace game2d