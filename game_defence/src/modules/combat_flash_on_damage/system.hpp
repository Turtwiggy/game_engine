#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_flash_sprite_system(entt::registry& r, uint64_t milliseconds_dt);

} // namespace game2d