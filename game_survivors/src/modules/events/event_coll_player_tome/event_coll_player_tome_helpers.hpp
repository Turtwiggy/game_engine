#pragma once

#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/collisions/resolve_collisions_helpers.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

std::vector<Stat>
get_stats_from_weapon_behaviour(entt::registry& r, const WeaponBehaviour behaviour);

void
handle_player_enter_tome(entt::registry& r, const OnCollisionEnter& evt);

} // namespace game2d