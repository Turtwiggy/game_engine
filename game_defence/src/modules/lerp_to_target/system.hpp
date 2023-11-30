#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_set_velocity_to_target_system(entt::registry& r, const float& dt);

} // namespace game2d