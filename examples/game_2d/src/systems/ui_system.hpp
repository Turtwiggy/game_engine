#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
init_ui_system(entt::registry& registry);

void
update_ui_system(entt::registry& registry, engine::Application& app);

} // namespace game2d