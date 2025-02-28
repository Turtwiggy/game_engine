#pragma once

#include "modules/core_collisions/resolve_collisions_helpers.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
handle_player_enter_hp(entt::registry& r, const OnCollisionEnter& evt);

} // namespace game2d