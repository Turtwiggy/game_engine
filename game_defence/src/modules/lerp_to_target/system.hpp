#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_lerp_to_target_system(entt::registry& r, const uint64_t& milliseconds_dt);

} // namespace game2d