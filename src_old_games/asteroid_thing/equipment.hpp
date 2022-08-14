#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct EquipmentComponent
{
  bool placeholder = true;
};

enum class EquipmentSlot
{
  HEAD,
  BODY,
  HAND_L,
  HAND_R,

  COUNT,
};

enum class Equipment
{
  POTION,
  GUN,
  SWORD,

  COUNT,
}

struct EqippableComponent
{
  EquipmentSlot slot;
};

struct PotionComponent
{
  int use_cooldown = 1;
  int heal_amount = 1;
};

entt::entity
create_potion(entt::registry& r);

// struct GunComponent
// {
//   int use_cooldown = 1;
//   int damage = 1;
//   int projectiles = 1;
//   int ammo = 1;
//   float fire_rate = 1.0f;
//   bool infinite_ammo = false;
// };

// struct SwordComponent
// {
//   int use_cooldown = 1;
//   int damage = 1;
//   int weapon_radius = 30;
//   float attack_rate = 0.15f;
// };

// struct TurretComponent
// {
//   float time_between_shots = 1.0f;
//   float time_since_last_shot = 0.0f;
//   float bullet_speed = 500.0f;
// };

// entt::entity
// create_turret(entt::registry& r);

// entt::entity
// create_gun(entt::registry& r);

// entt::entity
// create_sword(entt::registry& r);

} // namespace game2d
