#pragma once

#include <optional>

namespace game2d {

enum class Weapon
{
  sniper,
  shotgun,
  grunt,

  count,
};

struct AttackComponent
{
  int damage = 5;
};

struct RangeComponent
{
  int range = 1;
};

struct HealthComponent
{
  int hp = 100;
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

struct FlashOnDamageComponent
{
  bool started = false;
  int milliseconds_left = 200;
};

} // namespace game2d