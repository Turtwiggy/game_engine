#pragma once

#include <entt/fwd.hpp>

#include "modules/actors/actor_weapon/weapon_components.hpp"

namespace game2d {

entt::entity
spawn_weapon(entt::registry& r,
             const entt::entity player_e,
             const Weapon_OnDiskData& w_data,
             std::string key = "weapon_boat_default");

SINGLE_Weapons
load_weapons(entt::registry& r, std::string directory);

std::vector<entt::entity>
get_weapons(entt::registry& r, entt::entity player_e);

} // namespace game2d