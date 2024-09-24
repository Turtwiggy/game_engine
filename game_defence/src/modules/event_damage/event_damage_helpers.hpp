#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct DamageEvent
{
  entt::entity from;
  entt::entity to;
  int amount = 0;
};

void
handle_damage_event(entt::registry& r, const DamageEvent& evt);

} // namespace game2d