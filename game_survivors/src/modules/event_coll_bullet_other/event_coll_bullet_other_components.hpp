#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct BulletComponent
{
  bool placeholder = true;
};

struct BulletDamage
{
  int damage = 10;
};

struct BulletPierce
{
  int pierce = 1;
  int pierced = 0;
};

struct BulletSpeed
{
  int speed = 250;
};

struct BulletKnockback
{
  int knockback_force = 50;
};

struct WeaponFirerate
{
  float seconds_between_shots = 0.5f;
};

// How many bullets to fire every time a bullet is fired?
struct WeaponProjectiles
{
  int projectiles = 1;
};

// Given you're firing 1+ bullets,
// and you're firing in a given direction,
// what angle to add between the bullets?
struct WeaponSpread
{
  float angle_between_bullets_deg = 30;
};

} // namespace game2d