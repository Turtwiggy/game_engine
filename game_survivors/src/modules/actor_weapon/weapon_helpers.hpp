#pragma once

#include <entt/fwd.hpp>

#include "modules/actor_weapon/weapon_components.hpp"

namespace game2d {

entt::entity
spawn_weapon(entt::registry& r, const WeaponData& w_data, std::string key = "weapon_boat_default");

} // namespace game2d