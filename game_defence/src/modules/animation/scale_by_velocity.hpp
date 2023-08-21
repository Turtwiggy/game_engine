#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_scale_by_velocity_system(entt::registry& r, float dt);

} // namespace game2d