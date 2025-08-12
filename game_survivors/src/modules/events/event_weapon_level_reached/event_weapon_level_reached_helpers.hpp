#pragma once

#include "event_weapon_level_reached_components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/systems/system_weapon_upgrade/weapon_upgrade_components.hpp"

#include <entt/fwd.hpp>
#include <vector>

namespace game2d {

std::vector<Stat>
get_stats_from_weapon_behaviour(entt::registry& r, const WeaponBehaviour behaviour);

std::vector<WeaponUpgrade_OnDiskData>
get_upgrades_from_weapon_key(entt::registry& r, const std::string weapon_key);

void
handle_weapon_level_reached_event(entt::registry& r, const WeaponLevelReachedEvent& evt);

} // namespace game2d