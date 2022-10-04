#pragma once

// other project headers
#include <entt/entt.hpp>

namespace game2d {

void
init_camera_system(entt::registry& registry);

void
update_camera_system(entt::registry& registry, float dt);

} // namespace game2d
