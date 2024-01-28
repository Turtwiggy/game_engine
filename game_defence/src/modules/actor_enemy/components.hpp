#pragma once

namespace game2d {

enum class EnemyState
{
  CHASING,
  MELEE_ATTACKING,
  RANGED_ATTACKING,
};

struct RangedComponent
{
  // Distance to attack within
  float distance2 = (16 * 16 + 16 * 16) * 100;
};

struct MeleeComponent
{
  // Distance to attack within
  float distance2 = 16 * 16 + 16 * 16;
};

struct EnemyComponent
{
  EnemyState state = EnemyState::CHASING;

  // attack state
  float attack_speed = 3.0f;
  float attack_percent = 0.0f;
  bool has_applied_damage = false;
};

} // namespace game2d