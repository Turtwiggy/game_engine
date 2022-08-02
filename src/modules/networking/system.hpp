#pragma once

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
init_networking_system(entt::registry& registry);

void
update_networking_system(entt::registry& r, uint64_t milliseconds_dt);

} // namespace game2d
