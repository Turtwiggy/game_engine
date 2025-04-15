#pragma once

#include "modules/core/collisions/resolve_collisions_helpers.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_player_enemy_coll_enter(entt::registry& r, const OnCollisionEnter& evt);

void
handle_player_enemy_coll_exit(entt::registry& r, const OnCollisionExit& evt);

} // namespace game2d