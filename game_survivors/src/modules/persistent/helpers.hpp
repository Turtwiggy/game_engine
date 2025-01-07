#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
save_level(entt::registry& r, int level, bool complete);

bool
get_level_complete(entt::registry& r, int level);

} // namespace game2d