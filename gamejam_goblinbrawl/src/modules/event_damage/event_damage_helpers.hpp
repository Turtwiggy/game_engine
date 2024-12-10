#pragma once

#include "modules/event_damage/event_damage_components.hpp"

#include <entt/entt.hpp>

namespace game2d {

int
calculate_damage_to_take(entt::registry& r, entt::entity e, int amount, const DamageType& type);

void
handle_damage_event_take_damage(entt::registry& r, const DamageEvent& evt);

void
handle_damage_event_add_to_queue(entt::registry& r, const DamageEvent& evt);

} // namespace game2d