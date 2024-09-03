#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_combat_scale_on_hit_system(entt::registry& r, const float dt);

} // namespace game2d