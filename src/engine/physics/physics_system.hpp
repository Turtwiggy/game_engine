#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
update_physics_system(entt::registry& r, const uint64_t ms_dt);

} // namespace game2d