#pragma once

#include "app/game_window.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_camera_system(engine::GameWindow& window, entt::registry& r, const float dt);

} // namespace
