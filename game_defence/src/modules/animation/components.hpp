#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct ScaleTransformByVelocity
{
  bool placeholder = true;
};

struct RotatedSquare
{
  float theta = 0.0f;
  int unrotated_w = 0;
  int unrotated_h = 0;
  glm::vec3 center;
  std::vector<glm::vec3> points;
};

struct PlayerCursor
{
  bool placeholder = true;
};

} // namespace game2d