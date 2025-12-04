#pragma once

#include "modules/core/collisions/resolve_collisions_helpers.hpp"

#include <entt/fwd.hpp>

namespace game2d {

void
handle_player_enter_tome(entt::registry& r, const OnCollisionEnter& evt);

} // namespace game2d