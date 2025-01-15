#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_enemy_projectile_system(entt::registry& r);

void
add_projectile_enemy_components(entt::registry& r, entt::entity e);

} // namespace game2d