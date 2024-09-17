#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_fov_system(entt::registry& r, const glm::ivec2& mouse_pos);

} // namespace game2d