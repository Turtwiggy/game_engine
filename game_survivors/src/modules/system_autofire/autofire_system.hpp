#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_autofire_system(entt::registry& r, glm::vec2 mouse_pos);

} // namespace game2d