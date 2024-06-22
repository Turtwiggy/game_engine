#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_actor_bodypart_legs_system(entt::registry& r, const float dt, const glm::ivec2& mouse_position);

} // namespace game2d