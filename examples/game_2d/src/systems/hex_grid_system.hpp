#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
init_hex_grid_system(entt::registry& registry, glm::vec2 screen_wh);

void
update_hex_grid_system(entt::registry& registry, float dt);

} // namespace game2d