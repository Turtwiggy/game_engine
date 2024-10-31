#pragma once

#include <entt/entt.hpp>

namespace game2d {

void
update_move_player_on_map_system(entt::registry& r, uint64_t ms_dt);

} // namespace game2d