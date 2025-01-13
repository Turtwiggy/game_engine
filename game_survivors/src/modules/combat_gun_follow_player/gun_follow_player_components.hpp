#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct WeaponComponent
{
  float offset_amount = 32.0;
  float recoil_regain_speed = 100.0f;
  float recoil_amount = 5.0;
  float recoil_amount_max = 10.0;
};

struct GunStaticTargetComponent
{
  glm::ivec2 target{ 0, 0 };
};

// attached to e.g. walls, enemies
// struct DestroyBulletOnCollison
// {
//   bool placeholder = true;
// };

// struct RequestParticleOnCollision
// {
//   bool placeholder = true;
// };

// struct BulletComponent
// {
//   BulletType type = BulletType::DEFAULT;

//   bool bounce_bullet_on_wall_collision = false;
// };

} // namespace game2d