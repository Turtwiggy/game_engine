#pragma once

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

} // namespace game2d