#pragma once

#include <entt/fwd.hpp>

namespace game2d {

//
// these are traits that go beyond the "upgradeablestat" & do something unique
// for example shoot bullets behind every X shots
//

enum class WeaponDamageType
{
  KINETIC,
  FIRE,
  ICE,
  POISON,
  SHOCK,
};

enum class WeaponBehaviour
{
  // done
  SPLINTER,
  SHOOT_FANFIRE_ON_LAST_BULLET,
  MEGABULLET,
  SHOOT_BULLETS_OPPOSITE_DIRECTION,

  // todo
  HEAVY_PISTOL_CRIT_UPGRADE,
  CHANGE_DAMAGE_TO_KINETIC,
  CHANGE_DAMAGE_TO_FIRE,
  CHANGE_DAMAGE_TO_ICE,
  CHANGE_DAMAGE_TO_POISON,
  CHANGE_DAMAGE_TO_SHOCK,

  count,
};

struct WeaponBehaviourComponent
{
  std::unordered_set<WeaponBehaviour> traits;
};

} // namespace game2d