#pragma once

#include "entt/entt.hpp"

namespace game2d {

void
update_turret_system(entt::registry& r, const uint64_t& ms_dt);

} // namespace game2d