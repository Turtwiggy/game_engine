#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

void
init_hex_grid_system(entt::registry& registry);

void
update_hex_grid_system(entt::registry& registry, engine::Application& app, float dt);

} // namespace game2d