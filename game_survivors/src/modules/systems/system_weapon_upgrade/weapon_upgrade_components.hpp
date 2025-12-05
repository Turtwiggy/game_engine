#pragma once

#include <entt/fwd.hpp>

namespace game2d {

//
// these are traits that go beyond the "upgradeablestat" & do something unique
// for example shoot bullets behind every X shots
//

enum class WeaponBehaviour
{
  MEGABULLET,
  DOUBLE_PROJECTILES,
  SHOOT_BULLETS_OPPOSITE_DIRECTION,

  SHOOT_FANFIRE_ON_LAST_BULLET,
  SHOOT_FIREBALL,

  // general behaviours
  CHANGE_DAMAGE_TO_FIRE,
  CHANGE_DAMAGE_TO_ICE,

  // stats
  CLIPSIZE_UPGRADE, // increases reload time
  CRIT_UPGRADE,
  DAMAGE_UPGRADE,
  FIRERATE_UPGRADE,

  // turret
  TURRET_FOLLOW_PLAYER,
  TURRET_EXPLODE_ON_DEATH,

  // done
  // SPLINTER, // splinter on death

  // todo
  CHANGE_DAMAGE_TO_KINETIC,
  CHANGE_DAMAGE_TO_POISON,
  CHANGE_DAMAGE_TO_SHOCK,

  count,
};

struct WeaponBehaviourComponent
{
  std::unordered_set<WeaponBehaviour> behaviours;
};

} // namespace game2d