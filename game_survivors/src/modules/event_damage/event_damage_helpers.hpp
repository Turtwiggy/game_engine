#pragma once

#include "modules/event_damage/event_damage_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

float
calculate_damage_to_take(entt::registry& r, const DamageEvent& evt);

void
handle_damage_event_take_damage(entt::registry& r, const DamageEvent& evt);

} // namespace game2d