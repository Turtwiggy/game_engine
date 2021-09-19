#pragma once

// engine headers
#include "engine/application.hpp"

// helpers
#include "helpers/renderers/batch_triangle.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

// loosely based on:
// https://catlikecoding.com/unity/tutorials/hex-map/part-1/

void
init_hex_grid_system(entt::registry& registry);

void
update_hex_grid_system(entt::registry& registry, engine::Application& app, float dt);

}; // namespace game2d