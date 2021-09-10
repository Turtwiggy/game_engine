#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
init_render_system(entt::registry& registry, const glm::ivec2& screen_wh);

void
update_render_system(entt::registry& registry);

} // namespace game2d
