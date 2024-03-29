#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_lifecycle_system(entt::registry& r, const uint64_t& milliseconds_dt);

} // namespace game2d