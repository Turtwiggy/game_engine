#pragma once

#include "modules/combat_projectiles/projectile_components.hpp"

#include <vector>

namespace game2d {

std::vector<float>
generate_angles(float dir, int bullets, float spread_rad);

WeaponDef
get_weapon_def(entt::registry& r, entt::entity par_e, entt::entity wep_e);

BulletDef
get_bullet_def(entt::registry& r, entt::entity par_e, entt::entity wep_e);

// puts an angle in the range [0, 2π]
float
clamp_axis(float angle);

// puts an angle in the range [-π, π]
float
normalize_axis(float angle);

float
clamp_angle(float rad_a, float rad_min, float rad_max);

} // namespace game2d