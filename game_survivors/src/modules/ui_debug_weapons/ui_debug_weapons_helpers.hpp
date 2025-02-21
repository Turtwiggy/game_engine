#pragma once

#include "modules/combat/combat_helpers.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
equip_weapon(entt::registry& r, const WeaponData& wep);

} // namespace game2d