#pragma once

#include "actors.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct WeaponBulletTypeToSpawnComponent
{
  EntityType bullet_type = EntityType::bullet_default;
  float bullet_damage = 3.0f;
  float bullet_speed = 100.0f;
};

struct ShotgunComponent
{
  int bullets_to_spawn = 3;
};

} // namespace game2d