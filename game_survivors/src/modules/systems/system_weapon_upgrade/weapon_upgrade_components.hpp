#pragma once

#include <entt/fwd.hpp>

namespace game2d {

//
// these are traits that go beyond the "upgradeablestat" & do something unique
// for example shoot bullets behind every X shots
//

enum class WeaponBehaviour
{
  // done
  SPLINTER,

  // heavy pistol upgrades
  MEGABULLET,
  SHOOT_BULLETS_OPPOSITE_DIRECTION,
  HEAVY_PISTOL_CRIT_UPGRADE,
  SHOOT_FANFIRE_ON_LAST_BULLET,
  CHANGE_DAMAGE_TO_FIRE,

  // todo
  CHANGE_DAMAGE_TO_KINETIC,
  CHANGE_DAMAGE_TO_ICE,
  CHANGE_DAMAGE_TO_POISON,
  CHANGE_DAMAGE_TO_SHOCK,

  count,
};

struct WeaponBehaviourComponent
{
  std::unordered_set<WeaponBehaviour> behaviours;
};

} // namespace game2d