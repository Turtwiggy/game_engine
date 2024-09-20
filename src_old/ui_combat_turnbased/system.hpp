#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_ui_combat_turnbased_system(entt::registry& r, const glm::ivec2& input_mouse_pos);

} // namespace game2d