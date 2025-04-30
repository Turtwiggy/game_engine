#include "pch.hpp"

#include "aabb.hpp"

namespace game2d {

glm::vec2
AABB::center()
{
  return { tl.x + wh.x * 0.5f, tl.y + wh.y * 0.5f };
};

bool
contains(const AABB aabb, const glm::vec2 point)
{
  const auto xmin = aabb.tl.x;
  const auto xmax = aabb.tl.x + aabb.wh.x;
  const auto ymin = aabb.tl.y;
  const auto ymax = aabb.tl.y + aabb.wh.y;
  return (point.x >= xmin) && (point.x <= xmax) && (point.y >= ymin) && (point.y <= ymax);
}

} // namespace game2d