#include "pch.hpp"

#include "collision.hpp"

namespace game2d {

bool
crossing_number_algorithm__point_is_inside(const glm::vec2& point, const std::vector<Edge>& polygon)
{
  int intersections = 0;

  for (int i = 0; i < polygon.size(); i++) {
    const auto a = polygon[i].a;
    const auto b = polygon[i].b;
    const float x1 = (float)a.x;
    const float y1 = (float)a.y;
    const float x2 = (float)b.x;
    const float y2 = (float)b.y;

    if ((y1 <= point.y && y2 > point.y) || (y2 <= point.y && y1 > point.y)) {

      // Calculate the x-coordinate where the edge intersects the horizontal line
      float intersection_x = x1 + (point.y - y1) * (x2 - x1) / (y2 - y1);

      // If the intersection is to the right of the point, count it as a crossing
      if (intersection_x > point.x)
        intersections++;
    }
  }

  // If the number of intersections is odd, the point is inside
  return intersections % 2 == 1;
};

} // namespace game2d