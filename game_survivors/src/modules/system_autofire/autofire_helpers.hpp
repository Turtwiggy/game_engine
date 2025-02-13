#pragma once

#include "modules/combat_projectiles/projectile_components.hpp"

#include <vector>

namespace game2d {

std::vector<float>
generate_angles(const float dir, const int bullets, const float spread_rad);

WeaponDef
get_weapon_def(entt::registry& r, entt::entity par_e, entt::entity wep_e);

BulletDef
get_bullet_def(entt::registry& r, entt::entity par_e, entt::entity wep_e);

} // namespace game2d