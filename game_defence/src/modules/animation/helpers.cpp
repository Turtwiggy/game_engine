#include "helpers.hpp"

#include "maths/maths.hpp"
#include "modules/physics/helpers.hpp"

namespace game2d {

// generate_aabb IS BROKEN

AABB
generate_aabb(const RotatedSquare& square)
{
  AABB new_aabb;
  new_aabb.x.l = square.bl.x;
  new_aabb.x.r = square.bl.x;
  new_aabb.y.t = square.tl.y;
  new_aabb.y.b = square.tl.y;

  // What is the left-most x?
  new_aabb.x.l = square.tl.x < new_aabb.x.l ? square.tl.x : new_aabb.x.l;
  new_aabb.x.l = square.tr.x < new_aabb.x.l ? square.tr.x : new_aabb.x.l;
  new_aabb.x.l = square.br.x < new_aabb.x.l ? square.br.x : new_aabb.x.l;
  new_aabb.x.l = square.bl.x < new_aabb.x.l ? square.bl.x : new_aabb.x.l;

  // What is the right-most x?
  new_aabb.x.r = square.tl.x > new_aabb.x.r ? square.tl.x : new_aabb.x.r;
  new_aabb.x.r = square.tr.x > new_aabb.x.r ? square.tr.x : new_aabb.x.r;
  new_aabb.x.r = square.br.x > new_aabb.x.r ? square.br.x : new_aabb.x.r;
  new_aabb.x.r = square.bl.x > new_aabb.x.r ? square.bl.x : new_aabb.x.r;

  // What is the top-most y?
  new_aabb.y.t = square.tl.y > new_aabb.y.t ? square.tl.y : new_aabb.y.t;
  new_aabb.y.t = square.tr.y > new_aabb.y.t ? square.tr.y : new_aabb.y.t;
  new_aabb.y.t = square.br.y > new_aabb.y.t ? square.br.y : new_aabb.y.t;
  new_aabb.y.t = square.bl.y > new_aabb.y.t ? square.bl.y : new_aabb.y.t;

  // What is the bottom-most y?
  new_aabb.y.b = square.tl.y < new_aabb.y.b ? square.tl.y : new_aabb.y.b;
  new_aabb.y.b = square.tr.y < new_aabb.y.b ? square.tr.y : new_aabb.y.b;
  new_aabb.y.b = square.br.y < new_aabb.y.b ? square.br.y : new_aabb.y.b;
  new_aabb.y.b = square.bl.y < new_aabb.y.b ? square.bl.y : new_aabb.y.b;

  return new_aabb;
};

struct Edge
{
  glm::ivec2 a;
  glm::ivec2 b;
};

bool
rotated_squares_collide(const RotatedSquare& a, const RotatedSquare& b)
{
  // Broadphase: Check AABB
  // if (!collide(generate_aabb(a), generate_aabb(b)))
  //   return false;

  // Narrowphase:
  // alternative: GJK algorithm?

  const Edge edges_a[4] = { { a.tl, a.tr }, { a.tr, a.br }, { a.br, a.bl }, { a.bl, a.tl } };
  const Edge edges_b[4] = { { b.tl, b.tr }, { b.tr, b.br }, { b.br, b.bl }, { b.bl, b.tl } };

  const auto all_points_on_same_side = [](const Edge& e, const std::vector<glm::ivec2>& points) {
    const glm::vec2 eVec = { e.b.x - e.a.x, e.b.y - e.a.y };
    const glm::vec2 nVec = { -eVec.y, eVec.x }; // perpendicular to generate normal
    for (const glm::ivec2& point : points) {
      const glm::vec2 vector_to_point = { point.x - e.a.x, point.y - e.a.y };
      if (glm::dot(nVec, vector_to_point) < 0)
        return false;
    }
    return true;
  };

  for (int i = 0; i < 4; ++i) {
    const bool c0 = all_points_on_same_side(edges_a[i], { b.tl, b.tr, b.br, b.bl });
    const bool c1 = all_points_on_same_side(edges_b[i], { a.tl, a.tr, a.br, a.bl });
    if (c0 || c1) {
      return false; // No collision detected
    }
  }
  return true; // collision detected

  // One approach:
  // Test if any of the points is in the rectangle
  // This isnt a particularly quick approach
  // bool collision = false;
  // collision |= point_in_rectangle(a, b.tl);
  // if (collision)
  //   return true;
  // collision |= point_in_rectangle(a, b.tr);
  // if (collision)
  //   return true;
  // collision |= point_in_rectangle(a, b.br);
  // if (collision)
  //   return true;
  // collision |= point_in_rectangle(a, b.bl);
  // if (collision)
  //   return true;

  return false;
};

bool
point_in_rectangle(const RotatedSquare& square, glm::vec2 point)
{
  const auto& w = square.w;
  const auto& h = square.h;

  // center the point and rectangle to the origin
  const glm::vec2 tl = square.tl - square.center;
  const glm::vec2 tr = square.tr - square.center;
  const glm::vec2 br = square.br - square.center;
  const glm::vec2 bl = square.bl - square.center;
  const glm::vec2 t = point - square.center;

  // rotate the point around the origin
  const glm::ivec2 r = { t.x * std::cos(-square.theta) - t.y * std::sin(-square.theta),
                         t.x * std::sin(-square.theta) + t.y * std::cos(-square.theta) };

  return r.x >= -w / 2.0f && r.x <= w / 2.0f && r.y >= -h / 2.0f && r.y <= h / 2.0f;
};

RotatedSquare
transform_to_rotated_square(const TransformComponent& t)
{
  const auto& x = t.position.x;
  const auto& y = t.position.y;
  const auto& w = t.scale.x;
  const auto& h = t.scale.y;
  const auto& theta = t.rotation_radians.z;

  RotatedSquare square;
  square.theta = theta;
  square.center = { x, y };
  square.tl = engine::rotate_point({ -w / 2, h / 2 }, theta) + square.center;
  square.tr = engine::rotate_point({ w / 2, h / 2 }, theta) + square.center;
  square.br = engine::rotate_point({ w / 2, -h / 2 }, theta) + square.center;
  square.bl = engine::rotate_point({ -w / 2, -h / 2 }, theta) + square.center;
  square.w = w;
  square.h = h;

  return square;
}

} // namespace game2d