#pragma once

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
init(entt::registry& registry, glm::ivec2 screen_wh);

} // namespace game2d