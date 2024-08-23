#pragma once

#include "actors/base.hpp"

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
  EntityType bullet_type = EntityType::bullet_default;
};

struct BulletComponent
{
  bool destroy_bullet_on_wall_collision = false;
  bool destroy_bullet_on_actor_collision = true;
  bool bounce_bullet_on_wall_collision = false;
};

struct GunStaticTargetComponent
{
  std::optional<glm::vec2> target = std::nullopt;
};

} // namespace game2d