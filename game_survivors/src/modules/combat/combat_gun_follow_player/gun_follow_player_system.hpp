#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

void
update_gun_follow_player_system(entt::registry& r, const glm::vec2 mouse_pos, const float dt);

} // namespace game2d