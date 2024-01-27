#pragma once

#include "glm/glm.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
update_camera_system(entt::registry& r, const float dt, const glm::ivec2& mouse_pos);

} // namespace game2d
