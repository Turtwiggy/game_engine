#pragma once

#include <entt/entt.hpp>

namespace game2d {

// this could probably be replaced by a shader
// but the colliders would not work?

void
update_wiggle_up_and_down_system(entt::registry& r, float dt);

} // namespace game2d