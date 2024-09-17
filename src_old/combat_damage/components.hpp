#pragma once

#include <cstdint>
#include <entt/entt.hpp>

#include <vector>

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

struct TargetInfo
{
  entt::entity e = entt::null;
  int distance2 = std::numeric_limits<int>::max();
};

// struct RangeComponent
// {
//   int range = 1;
// };

struct HoldAndReleaseShootComponent
{
  bool in_windup = false;

  // keep a track of arrows created
  // so you can map them to
  std::vector<entt::entity> my_bullets;
};

// struct BurnableComponent
// {
//   bool placeholder = true;
// };

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

//
// When attacking,
// create a DealDamageRequest
//

struct DealDamageRequest
{
  entt::entity from;
  entt::entity to;
};

struct HasWeaponComponent
{
  entt::entity instance = entt::null;
};

} // namespace game2d