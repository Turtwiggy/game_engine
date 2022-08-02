#pragma once

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
update_move_objects_system(entt::registry& registry, uint64_t milliseconds_dt);

} // namespace game2d