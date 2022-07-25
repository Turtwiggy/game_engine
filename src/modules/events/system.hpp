#pragma once

// engine headers
#include "engine/app/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
init_input_system(entt::registry& registry);

void
update_input_system(entt::registry& registry, engine::Application& app);

} // namespace game2d