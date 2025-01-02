#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_parallax_mouse_system(entt::registry& r, const float dt, const glm::ivec2 mouse_pos);

} // namespace game2d