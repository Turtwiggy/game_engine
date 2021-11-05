#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
update_ai_head_to_random_point_system(entt::registry& registry, engine::Application& app, float dt);

} // namespace game2d