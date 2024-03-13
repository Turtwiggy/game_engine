#pragma once

#include "entt/entt.hpp"
#include "glm/glm.hpp"

namespace game2d {

void
update_selected_interactions_system(entt::registry& r, const glm::ivec2& mouse_pos, const float dt);

} // namespace game2d