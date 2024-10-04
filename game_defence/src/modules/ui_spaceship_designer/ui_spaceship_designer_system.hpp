#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_ui_spaceship_designer_system(entt::registry& r, const glm::vec2& mouse_pos, const float dt);

} // namespace game2d