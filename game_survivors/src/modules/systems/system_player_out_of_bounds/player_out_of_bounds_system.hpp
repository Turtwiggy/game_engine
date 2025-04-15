#pragma once

#include <entt/fwd.hpp>

namespace game2d {

void
update_player_out_of_bounds_system(entt::registry& r, const float dt);

} // namespace game2d