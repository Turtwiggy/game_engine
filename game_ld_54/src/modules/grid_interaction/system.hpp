#pragma once

#include "entt/entt.hpp"

namespace game2d {

void
update_grid_interaction_system(entt::registry& r, const glm::ivec2& mouse_pos);

} // namespace game2d