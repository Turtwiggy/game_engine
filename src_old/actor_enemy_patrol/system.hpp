#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_actor_enemy_patrol_system(entt::registry& r, const float dt);

} // namespace game2d