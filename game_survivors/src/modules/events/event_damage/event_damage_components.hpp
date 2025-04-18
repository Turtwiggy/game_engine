#pragma once

#include "modules/actors/actor_weapon/weapon_components.hpp"

#include <entt/fwd.hpp>

namespace game2d {

struct DamageEvent
{
  entt::entity from = entt::null;
  entt::entity to = entt::null;
  float amount = 0.0f;
  WEAPON_DAMAGE type;
};

} // namespace game2d