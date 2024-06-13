#include "line.hpp"

#include "maths/maths.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"

namespace game2d {

LineInfo
generate_line(const glm::ivec2& a, const glm::ivec2& b, const int width)
{
  const glm::vec2 raw_dir = { b.x - a.x, b.y - a.y };
  const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);
  const float angle = engine::dir_to_angle_radians(nrm_dir) + engine::HALF_PI;

  LineInfo line;
  line.position = { (a.x + a.x + raw_dir.x) / 2, (a.y + a.y + raw_dir.y) / 2 };
  line.rotation = angle;
  line.scale.x = width;
  line.scale.y = sqrt(pow(raw_dir.x, 2) + pow(raw_dir.y, 2));
  return line;
};

void
set_transform_with_line(TransformComponent& t, const LineInfo& line)
{
  t.position = { line.position.x, line.position.y, 0 };
  t.scale = { line.scale.x, line.scale.y, 0 };
  t.rotation_radians.z = line.rotation;
};

void
set_transform_with_line(entt::registry& r, const entt::entity& e, const LineInfo& li)
{
  auto& t = r.get<TransformComponent>(e);
  set_transform_with_line(t, li);
};

void
game2d::set_position_and_size_with_line(entt::registry& r, const entt::entity& e, const LineInfo& li)
{
  if (auto* aabb = r.try_get<AABB>(e)) {
    aabb->center = { li.position.x, li.position.y };
    aabb->size = { li.scale.x, li.scale.y };
  }
  set_transform_with_line(r, e, li);
}

std::vector<std::pair<int, int>>
create_line(int r0, int c0, int r1, int c1)
{
  std::vector<std::pair<int, int>> results;

  int r = r0;
  int c = c0;
  int dr = glm::abs(r1 - r0);
  int dc = glm::abs(c1 - c0);
  int sc = c1 - c > 0 ? 1 : -1;
  int sr = r1 - r > 0 ? 1 : -1;

  int steep = 0;
  if (dr > dc) {
    steep = 1;
    std::swap(c, r);
    std::swap(dc, dr);
    std::swap(sc, sr);
  }
  int d = (2 * dr) - dc;

  std::pair<int, int> res;

  for (int i = 0; i < dc; i++) {
    if (steep) {
      res.first = c;
      res.second = r;
      results.push_back(res);
    } else {
      res.first = r;
      res.second = c;
      results.push_back(res);
    }
    while (d >= 0) {
      r = r + sr;
      d = d - (2 * dc);
    }
    c = c + sc;
    d = d + (2 * dr);
  }

  res.first = r1;
  res.second = c1;
  results.push_back(res);

  return results;
};

// Function to calculate the intersection point of two lines
// Returns a tuple with the intersection point (x, y)
std::optional<glm::ivec2>
line_intersection(const int x1, // p0
                  const int y1,
                  const int x2, // p1
                  const int y2,
                  const int x3, // p2
                  const int y3,
                  const int x4, // p3
                  const int y4)
{
  const auto create_aabb = [](const glm::ivec2& tl, const int width, const int height) -> AABB {
    AABB aabb;
    aabb.center = { tl.x + (width / 2.0f), tl.y + (height / 2.0f) };
    aabb.size = { glm::abs(width), glm::abs(height) };
    return aabb;
  };

  // Calculate determinants
  const int det = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  const int det_x = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
  const int det_y = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);

  // If determinant is zero, lines are parallel or coincident
  // If determinant is negative... not intested in future collisions.
  if (det == 0)
    return std::nullopt;

  // Calculate intersection point
  const float intersection_x = det_x / static_cast<float>(det);
  const float intersection_y = det_y / static_cast<float>(det);
  const glm::ivec2 intersection = { static_cast<int>(intersection_x), static_cast<int>(intersection_y) };

  // check the intersection lies within the line segments.
  AABB point;
  point.center = intersection;
  point.size = { 1, 1 };
  // ImGui::Text("Point. Center: %i %i. Size: %i %i", point.center.x, point.center.y, point.size.x, point.size.y);

  const int l0 = glm::min(x1, x2);
  const int r0 = glm::max(x1, x2);
  const int t0 = glm::min(y1, y2);
  const int b0 = glm::max(y1, y2);
  AABB line0 = create_aabb({ l0, t0 }, glm::abs(r0 - l0), glm::abs(b0 - t0));
  line0.size.x = line0.size.x == 0 ? 1 : line0.size.x;
  line0.size.y = line0.size.y == 0 ? 1 : line0.size.y;
  // ImGui::Text("line0. Center: %i %i. Size: %i %i", line0.center.x, line0.center.y, line0.size.x, line0.size.y);

  const int l = glm::min(x3, x4);
  const int r = glm::max(x3, x4);
  const int t = glm::min(y3, y4);
  const int b = glm::max(y3, y4);
  AABB line1 = create_aabb({ l, t }, glm::abs(r - l), glm::abs(b - t));
  line1.size.x = line1.size.x == 0 ? 1 : line1.size.x;
  line1.size.y = line1.size.y == 0 ? 1 : line1.size.y;
  // ImGui::Text("line1. Center: %i %i. Size: %i %i", line1.center.x, line1.center.y, line1.size.x, line1.size.y);

  const bool coll_a = collide(point, line0);
  const bool coll_b = collide(point, line1);
  if (coll_a && coll_b)
    return intersection;
  return std::nullopt;
}

std::optional<glm::ivec2>
line_intersection(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3)
{
  return line_intersection(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
};

glm::ivec2
get_size_from_line(const Line& l, int line_width)
{
  int width = glm::abs(l.p0.x - l.p1.x);
  int height = glm::abs(l.p0.y - l.p1.y);

  // set a line width
  if (l.p0.x != l.p1.x && height == 0)
    height = line_width;
  if (l.p0.y != l.p1.y && width == 0)
    width = line_width;

  return { width, height };
}

glm::ivec2
get_center_from_line(const Line& l)
{
  return { (l.p0.x + l.p1.x) / 2.0f, (l.p0.y + l.p1.y) / 2.0f };
}

std::vector<std::pair<Line, glm::ivec2>>
get_line_collisions(const std::vector<Line>& lines, const Line& l)
{
  std::vector<std::pair<Line, glm::ivec2>> collisions;

  for (const Line& other_l : lines) {

#if defined(_DEBUG)
    // sanity checks
    assert(l.p1.x >= l.p0.x);
    assert(l.p1.y >= l.p0.y);
    assert(other_l.p1.x >= other_l.p0.x);
    assert(other_l.p1.y >= other_l.p0.y);
#endif

    const auto intersection = line_intersection(l.p0, l.p1, other_l.p0, other_l.p1);
    if (intersection.has_value())
      collisions.push_back({ other_l, intersection.value() });
  }

  return collisions;
};

bool
is_horizontal(const Line& l)
{
  return l.p0.y == l.p1.y;
};

void
make_p1_bigger_point(Line& l)
{
  const bool horizontal = is_horizontal(l);
  if (horizontal && l.p0.x > l.p1.x)
    std::swap(l.p0.x, l.p1.x);
  if (!horizontal && l.p0.y > l.p1.y)
    std::swap(l.p0.y, l.p1.y);
};

} // namespace game2d