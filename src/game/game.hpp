#pragma once

// other lib headers
#include "entt/entt.hpp"
#include "glm/glm.hpp"

namespace game2d {

void
init(entt::registry& registry);

void
fixed_update(entt::registry& registry, uint64_t milliseconds_dt);

void
update(entt::registry& registry, float dt);

} // namespace game2d