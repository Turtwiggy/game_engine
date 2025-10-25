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

std::vector<std::string>
get_aquired_upgrades(entt::registry& r,
                     const std::vector<WeaponUpgrade_OnDiskData>& weapon_upgrades_data,
                     entt::entity wep_e);

std::string
get_wb_key_from_upgrade_key(entt::registry& r, const std::string& u_key);

std::string
get_display_key_from_upgrade_key(entt::registry& r, const std::string& u_key);

void
handle_weapon_level_reached_event(entt::registry& r, const WeaponLevelReachedEvent& evt);

} // namespace game2d