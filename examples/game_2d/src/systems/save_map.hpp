#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
update_save_map_system(entt::registry& registry, engine::Application& app, float dt);

}; // namespace game2d