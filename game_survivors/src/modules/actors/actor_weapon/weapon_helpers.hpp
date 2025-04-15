#pragma once

#include <entt/fwd.hpp>

#include "modules/actors/actor_weapon/weapon_components.hpp"

namespace game2d {

entt::entity
spawn_weapon(entt::registry& r, const Weapon_OnDiskData& w_data, std::string key = "weapon_boat_default");

} // namespace game2d