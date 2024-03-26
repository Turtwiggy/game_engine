#pragma once

#include "actors.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct WeaponBulletTypeToSpawnComponent
{
  EntityType bullet_type = EntityType::bullet_default;
  float bullet_damage = 3.0f;
  float bullet_speed = 200.0f;
};

struct ShotgunComponent
{
  int bullets_to_spawn = 3;

  float offset_amount = 25.0;

  float recoil_regain_speed = 100.0f;
  float recoil_amount = 0.0;
  float recoil_amount_max = 10.0;
};

struct BulletComponent
{
  bool placeholder = true;
};

} // namespace game2d