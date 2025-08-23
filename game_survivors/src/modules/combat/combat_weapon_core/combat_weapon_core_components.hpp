#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct WeaponFireRate
{
  float base_firerate = 2; // shots per second

  float seconds_between_shots_max = 0.5f;  // 1.0/firerate
  float seconds_between_shots_left = 0.0f; // the cooldown
};

struct WeaponReloadRate
{
  float seconds_base_max = 0.5f;
  float seconds_cur = 0.0f;
};

// How many bullets to fire every time a bullet is fired?
struct WeaponProjectiles
{
  int projectiles = 1;
};

// Distance at which the weapon can fire.
struct WeaponRange
{
  float meters = 3.0f;
};

// Given you're firing 1+ bullets,
// and you're firing in a given direction,
// what angle to add between the bullets?
struct WeaponSpread
{
  float angle_between_bullets_deg = 30;
};

struct WeaponClipSize
{
  int bullets_max = 1;
  int bullets_cur = 0;
};

} // namespace game2d