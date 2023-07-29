#pragma once

#include "box2d/box2d.h"
#include <entt/entt.hpp>

namespace game2d {

void
update_physics_box2d_system(entt::registry& r, b2World& world, const uint64_t& milliseconds_dt);

} // namespace game2d