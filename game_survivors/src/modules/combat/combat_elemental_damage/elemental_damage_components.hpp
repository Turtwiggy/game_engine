#pragma once

#include <entt/fwd.hpp>

#include "modules/actors/actor_weapon/weapon_components.hpp"

namespace game2d {

struct TickDamageComponent
{
  float time_since_last_tick = 0.0f;
  float time_since_last_tick_max = 0.5f;

  // could do something better
  std::vector<std::pair<WEAPON_DAMAGE, float>> fire;
  std::vector<std::pair<WEAPON_DAMAGE, float>> ice;
  std::vector<std::pair<WEAPON_DAMAGE, float>> shock;
  std::vector<std::pair<WEAPON_DAMAGE, float>> poison;
};

} // namespace game2d