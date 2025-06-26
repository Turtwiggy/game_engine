#pragma once

namespace game2d {

struct ConvexShape
{
  // float theta = 0.0f;
  // int unrotated_w = 0;
  // int unrotated_h = 0;
  glm::vec3 center;
  std::vector<glm::vec3> points;
};

bool
gjk_collide(const ConvexShape& a, const ConvexShape& b);

} // namespace game2d