#pragma once

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
update_lifecycle_system(entt::registry& registry, float fixed_dt);

} // namespace game2d