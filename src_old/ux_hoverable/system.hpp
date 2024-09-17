#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
update_ux_hoverable(entt::registry& r, const glm::ivec2& mouse_pos);

};