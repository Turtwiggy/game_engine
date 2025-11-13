#pragma once

#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/combat/combat_weapon_type_area/combat_weapon_type_area_components.hpp"

#include <vector>

namespace game2d {

std::vector<float>
generate_angles(const float dir, const int bullets, const float spread_rad);

Weapon_OnDiskData
get_weapon_data(entt::registry& r, const std::string weapon_key);

WeaponDef
get_weapon_def(entt::registry& r, const entt::entity wep_e);

BulletDef
get_bullet_def(entt::registry& r, const entt::entity wep_e);

ElementalDef_ModifiersApplied
get_area_def(entt::registry& r, const entt::entity wep_e);

} // namespace game2d