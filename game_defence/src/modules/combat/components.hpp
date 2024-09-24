#pragma once

#include <entt/entt.hpp>

namespace game2d {

// e.g. added to barrels and destroyable objects
struct HealthComponent
{
  int max_hp = 100;
  int hp = 100;
};

// e.g. added to bullets
struct AttackComponent
{
  float damage = 5.0f;
};

// a shotgun can have 3 bullets. link the bullets
static uint64_t global_attack_id = 0;
struct AttackIdComponent
{
  uint64_t id = 0;
  AttackIdComponent() { id = global_attack_id++; }
};

struct DefenceComponent
{
  int armour = 0;
};

struct DefenceHitListComponent
{
  std::vector<uint64_t> attack_id_taken;
};

enum class AvailableTeams
{
  enemy,
  player,
  neutral,
  count
};

struct TeamComponent
{
  AvailableTeams team = AvailableTeams::neutral;
};

struct HasWeaponComponent
{
  entt::entity instance = entt::null;
};

} // namespace game2d