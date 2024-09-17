#pragma once

#include "glm/glm.hpp"

namespace game2d {

struct SetPositionAtDynamicTarget
{
  glm::vec2 offset{ 0, 0 };
};

struct SetRotationAsDynamicTarget
{
  float offset = 0.0f;
};

// discrete will defini
struct LerpToFixedTarget
{
  glm::vec2 a{ 0.0f, 0.0f };
  glm::vec2 b{ 0.0f, 0.0f };
  float t = 0.0f;
  float speed = 20.0f; // higher number = faster to destination
};

struct LerpToMovingTarget
{
  glm::vec2 a{ 0.0f, 0.0f };
  glm::vec2 b{ 0.0f, 0.0f };
  // float t = 0.0f; no t because continous
  float speed = 20.0f; // higher number = faster to destination
};

} // namespace game2d