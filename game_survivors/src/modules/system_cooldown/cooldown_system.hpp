#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
update_cooldown_system(entt::registry& r, const uint64_t milliseconds_dt);

};