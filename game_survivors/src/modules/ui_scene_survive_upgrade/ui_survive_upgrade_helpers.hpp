#pragma once

#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
generate_upgrades_for_players(entt::registry& r, SINGLE_LevelUpUI& ui_c);

bool
is_choosing_upgrade(entt::registry& r);

entt::entity
get_player_e_from_idx(entt::registry& r, int player_idx);

} // namespace game2d