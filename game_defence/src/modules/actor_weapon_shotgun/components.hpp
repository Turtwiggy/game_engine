#pragma once

#include "actors/bags/bullets.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

struct ShotgunComponent
{
  int bullets_to_spawn = 2;
  float offset_amount = 32.0;
  float recoil_regain_speed = 100.0f;
  float recoil_amount = 5.0;
  float recoil_amount_max = 10.0;

  // bullet_type doesnt belong on shotgun.
  // need something else. incase more guns.
  BulletType bullet_type = BulletType::DEFAULT;
};

struct BulletComponent
{
  BulletType type = BulletType::DEFAULT;

  bool bounce_bullet_on_wall_collision = false;
};

// attached to e.g. walls, enemies
struct DestroyBulletOnCollison
{
  bool placeholder = true;
};

struct GunStaticTargetComponent
{
  std::optional<glm::vec2> target = std::nullopt;
};

struct RequestParticleOnCollision
{
  bool placeholder = true;
};

} // namespace game2d