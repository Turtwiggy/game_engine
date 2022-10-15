#pragma once

#include <entt/entt.hpp>

// other lib headers
#include <glm/glm.hpp>

namespace game2d {

void
update_free_cursor(entt::registry& r, glm::ivec2 mouse_pos);

void
update_grid_cursor(entt::registry& r, glm::ivec2 mouse_pos);

} // namespace game2d