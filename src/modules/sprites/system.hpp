#pragma once

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
init_sprite_system(entt::registry& registry);

void
update_sprite_system(entt::registry& registry, float dt);

} // namespace game2d