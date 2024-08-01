#pragma once

#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace game2d {

struct RotatedSquare
{
  float theta = 0.0f;
  int unrotated_w = 0;
  int unrotated_h = 0;
  glm::vec3 center;
  std::vector<glm::vec3> points;
};

bool
gjk_squares_collide(const RotatedSquare& a, const RotatedSquare& b);

RotatedSquare
transform_to_rotated_square(const TransformComponent& t);

}; // namespace game2d