#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
update_move_objects_system(entt::registry& registry, engine::Application& app);

} // namespace game2d