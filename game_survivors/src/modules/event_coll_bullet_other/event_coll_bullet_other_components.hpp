#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct BulletComponent
{
  bool placeholder = true;
};

// when you collide with an enemy, reverse your vel
struct BulletBounce
{
  int bounces_left = 0;
};

struct BulletSize
{
  glm::vec2 size{ 6, 6 };
};

struct BulletCrit
{
  float crit_chance = 0;   // percent
  float crit_damage = 150; // percent * base damage
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
  float speed = 1.0f; // m/s
};

struct BulletKnockback
{
  int knockback_force = 0;
};

struct WeaponFireRate
{
  float base_firerate = 2; // shots per second

  float seconds_between_shots_max = 0.5f;  // 1.0/firerate
  float seconds_between_shots_left = 0.5f; // the cooldown
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
  int meters = 3;
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