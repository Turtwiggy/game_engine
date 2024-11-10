#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct DamageEvent
{
  entt::entity from;
  entt::entity to;
};

int
calculate_damage_to_take(entt::registry& r, entt::entity from_e, entt::entity to_e);

void
handle_damage_event(entt::registry& r, const DamageEvent& evt);

} // namespace game2d