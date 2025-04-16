#pragma once

#include "event_weapon_level_reached_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
handle_weapon_level_reached_event(entt::registry& r, const WeaponLevelReachedEvent& evt);

} // namespace game2d