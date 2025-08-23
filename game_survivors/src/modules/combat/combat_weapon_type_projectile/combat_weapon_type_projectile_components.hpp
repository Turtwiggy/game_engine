#pragma once

#include <entt/fwd.hpp>

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
  float knockback_force = 0.0f;
};

struct BulletLifetime
{
  float seconds = 3;
};

} // namespace game2d