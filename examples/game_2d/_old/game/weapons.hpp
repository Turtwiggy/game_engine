#pragma once

// engine headers
#include "engine/maths.hpp"

// game headers
#include "items.hpp"

namespace game2d {

// An "Attack" is basically a limiter that prevents collisions
// applying damage on every frame. This could end up being super weird.
struct Attack
{
private:
  static inline uint32_t global_attack_int_counter = 0;

public:
  uint32_t id = 0;

  int entity_weapon_owner_id; // player or enemy
  int entity_weapon_id;
  ShopItem weapon_type;
  int weapon_damage = 0;

  Attack(int parent, int weapon, ShopItem type, int damage)
    : entity_weapon_owner_id(parent)
    , entity_weapon_id(weapon)
    , weapon_type(type)
    , weapon_damage(damage)
  {
    id = ++Attack::global_attack_int_counter;
  };
};

struct WeaponStats
{
  int damage = 0;
};

struct MeleeWeaponStats : WeaponStats
{
  // slash stats
  float slash_attack_time = 0.15f;
  float weapon_radius = 30.0f;
  float weapon_angle_speed = engine::HALF_PI / 30.0f; // closer to 0 is faster
  float weapon_damping = 20.0f;
  float slash_attack_time_left = 0.0f;
  bool attack_left_to_right = true;
  glm::vec2 weapon_target_pos = { 0.0f, 0.0f };
  float weapon_current_angle = 0.0f;
};
struct RangedWeaponStats : WeaponStats
{
  int radius_offset_from_player = 14;
  bool infinite_ammo = true;
  float fire_rate_seconds_limit = 1.0f;
  int current_ammo = 20;
};

MeleeWeaponStats
create_shovel();

RangedWeaponStats
create_pistol();

RangedWeaponStats
create_shotgun();

RangedWeaponStats
create_machinegun();

}