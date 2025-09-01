#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct WeaponTypeAreaComponent
{
  bool placeholder = true;
};

struct AreaWeapon_Beams
{
  int beams = 1;
};

struct AreaWeapon_Size
{
  float size_x = 1.0f;
  float size_y = 1.0f;
};

struct AreaWeapon_StackDamage
{
  float damage_mul = 1.0f;
};

struct AreaWeapon_StackDuration
{
  float seconds = 1.0f;
};

struct AreaWeapon_StacksAppliedPerShot
{
  int stacks = 1;
};

struct AreaDef_ModifiersApplied
{
  int beams = 1;
  float size_x = 1.0f;
  float size_y = 1.0f;
  float stack_damage = 1.0f;
  float stack_duration = 1.0f;
  int stacks_per_shot = 1;
};

} // namespace game2d