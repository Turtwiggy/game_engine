#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
simulate(entt::registry& r, uint64_t milliseconds_dt);

} // namespace game2d