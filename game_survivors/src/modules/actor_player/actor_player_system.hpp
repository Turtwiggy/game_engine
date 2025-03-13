#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

void
update_player_controller_system(entt::registry& r, const uint64_t milliseconds_dt, const glm::ivec2& mouse_pos);

void
fixed_update_player_controller_system(entt::registry& r, const uint64_t milliseconds_dt, const glm::ivec2& mouse_pos);

} // namespace game2d