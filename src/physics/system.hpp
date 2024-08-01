#pragma once

#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace game2d {

void
update_physics_system(entt::registry& r, const uint64_t ms_dt);

} // namespace game2d