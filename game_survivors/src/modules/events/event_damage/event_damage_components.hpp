#pragma once

#include "modules/actors/actor_weapon/weapon_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct DamageEvent
{
  entt::entity from = entt::null;

  // to_parent does not have health fixture.
  // it contains
  // e.g. StatsModifierComponent
  // e.g. TransformComponent
  entt::entity to_parent = entt::null;

  // to_parent does have health fixture.
  // it contains
  // e.g. HealthComponent
  // it is possible that to_parent == to_fixture.
  entt::entity to_fixture = entt::null;

  // entt::entity to = entt::null;
  float amount = 0.0f;
  WEAPON_DAMAGE type;
};

} // namespace game2d