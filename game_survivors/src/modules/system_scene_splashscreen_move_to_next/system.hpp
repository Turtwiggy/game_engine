#pragma once

#include "engine/app/application.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
update_scene_splashscreen_move_to_next_system(engine::SINGLE_Application& app, entt::registry& r, const float dt);

} // namespace game2d