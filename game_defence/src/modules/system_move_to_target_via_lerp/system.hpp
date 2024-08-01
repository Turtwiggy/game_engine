#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_move_to_target_via_lerp(entt::registry& r, const float& dt);

} // namespace game2d