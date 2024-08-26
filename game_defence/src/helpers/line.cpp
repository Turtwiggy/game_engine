#include "line.hpp"

#include "actors/helpers.hpp"
#include "maths/maths.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"


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
  line.scale.x = static_cast<float>(width);
  line.scale.y = static_cast<float>(sqrt(pow(raw_dir.x, 2) + pow(raw_dir.y, 2)));
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
set_position_and_size_with_line(entt::registry& r, const entt::entity e, const LineInfo& li)
{
  set_position(r, e, li.position);
  set_size(r, e, li.scale);

  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = li.rotation;
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
// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
std::optional<glm::ivec2>
line_intersection(const int p0_x,
                  const int p0_y,
                  const int p1_x,
                  const int p1_y,
                  const int p2_x,
                  const int p2_y,
                  const int p3_x,
                  const int p3_y)
{
  const float s1_x = static_cast<float>(p1_x - p0_x);
  const float s1_y = static_cast<float>(p1_y - p0_y);
  const float s2_x = static_cast<float>(p3_x - p2_x);
  const float s2_y = static_cast<float>(p3_y - p2_y);

  const float s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
  const float t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

  if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
    // collision detected
    return glm::ivec2{
      static_cast<int>(p0_x + (t * s1_x)),
      static_cast<int>(p0_y + (t * s1_y)),
    };
  }
  return std::nullopt;
}

std::optional<glm::ivec2>
line_intersection(const glm::ivec2& p0, const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3)
{
  return line_intersection(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
};

glm::ivec2
get_size_from_line(const Line& l, float line_width)
{
  float width = glm::abs(l.p0.x - l.p1.x);
  float height = glm::abs(l.p0.y - l.p1.y);

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

std::vector<LineCollision>
get_line_collisions(const std::vector<Line>& lines, const Line& l)
{
  std::vector<LineCollision> collisions;

  for (const Line& wall_line : lines) {

#if defined(_DEBUG)
    // sanity checks
    assert(l.p1.x >= l.p0.x);
    assert(l.p1.y >= l.p0.y);
    assert(wall_line.p1.x >= wall_line.p0.x);
    assert(wall_line.p1.y >= wall_line.p0.y);
#endif

    const auto intersection = line_intersection(l.p0, l.p1, wall_line.p0, wall_line.p1);
    if (intersection.has_value()) {
      LineCollision coll;
      coll.line_a = wall_line;
      coll.line_b = l;
      coll.intersection = intersection.value();
      collisions.push_back(coll);
    }
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