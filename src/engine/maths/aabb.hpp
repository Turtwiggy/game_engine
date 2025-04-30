#pragma once

namespace game2d {

struct AABB
{
  glm::vec2 tl;
  glm::vec2 wh;
  glm::vec2 center();
};

bool
contains(const AABB aabb, const glm::vec2 point);

} // namespace game2d