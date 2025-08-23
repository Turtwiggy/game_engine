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
  SPLINTER, // splinter on death

  // cannon [heavy]
  MEGABULLET,
  SHOOT_BULLETS_OPPOSITE_DIRECTION,
  HEAVY_PISTOL_CRIT_UPGRADE,
  SHOOT_FANFIRE_ON_LAST_BULLET,
  // CHANGE_DAMAGE_TO_FIRE,

  // cannon [grapeshot]
  // MEGABULLET,
  DOUBLE_PROJECTILES,
  SHOTGUN_CLIPSIZE_UPGRADE,
  SHOTGUN_DAMAGE_UPGRADE,
  SHOTGUN_FIRERATE_UPGRADE,

  // cannon [gatling]
  SMG_DAMAGE_UPGRADE,
  SMG_CRIT_UPGRADE,
  SMG_FIREBALL_UPGRADE,
  SMG_KNOCKBACK_UPGRADE,
  SMG_CLIPSIZE_UPGRADE,

  // turret
  TURRET_FOLLOW_PLAYER,
  TURRET_SLOW_ENEMIES,
  TURRET_EXTRA_TURRETS_UPGRADE,
  TURRET_EXPLODE_ON_DEATH,
  // CHANGE_DAMAGE_TO_ICE,

  // general behaviours
  CHANGE_DAMAGE_TO_FIRE,
  CHANGE_DAMAGE_TO_ICE,

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