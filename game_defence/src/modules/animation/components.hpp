#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct SetTransformAngleToVelocity
{
  bool placeholder = true;
};

struct WiggleUpAndDown
{
  glm::ivec2 base_position;
  float time = 0.0f;
  float offset = 0.0f;

  float amplitude = 2.0f;
  float frequency = 5.0f;
};

struct RotateAroundEntity
{
  entt::entity e = entt::null;
  float distance = 1.0f;
  float rotate_speed = 1.0f;
  float theta = 0.0f; // 0 and 2PI
};

} // namespace game2d