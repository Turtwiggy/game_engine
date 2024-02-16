#include "gjk.hpp"

#include "maths/maths.hpp"

namespace game2d {

glm::vec3
triple_product(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
  return glm::cross(glm::cross(a, b), c);
}

glm::vec3
support(const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b, const glm::vec3& dir)
{
  const auto furthest_point = [](const auto& points, const glm::vec3& d) -> glm::vec3 {
    glm::vec3 furthest_point_so_far;
    float highest_dot_product_so_far = -FLT_MAX;

    for (const auto& point : points) {
      const float result = glm::dot(point, d);
      if (result > highest_dot_product_so_far) {
        highest_dot_product_so_far = result;
        furthest_point_so_far = point;
      }
    }
    return furthest_point_so_far;
  };

  const auto furthest_point_a = furthest_point(a, dir);
  const auto furthest_point_b = furthest_point(b, -dir);
  return furthest_point_a - furthest_point_b;
};

bool
line_case(const std::vector<glm::vec3>& simplex, glm::vec3& dir)
{
  const auto& B = simplex[0];
  const auto& A = simplex[1];

  const auto AB = B - A;
  const auto AO = glm::vec3{ 0, 0, 0 } - A;

  // known as the triple product
  const auto ABperp = triple_product(AB, AO, AB);

  dir = glm::normalize(ABperp);

  return false;
};

bool
triangle_case(std::vector<glm::vec3>& simplex, glm::vec3& dir)
{
  const auto& C = simplex[0];
  const auto& B = simplex[1];
  const auto& A = simplex[2];

  const auto AB = B - A;
  const auto AC = C - A;
  const auto AO = -A;

  {
    const auto ABperp = triple_product(AC, AB, AB);
    if (dot(ABperp, AO) > 0) {
      simplex.erase(simplex.begin()); // remove c
      dir = ABperp;
      return false;
    }
  }

  {
    const auto ACperp = triple_product(AB, AC, AC);
    if (dot(ACperp, AO) > 0) {
      simplex.erase(simplex.begin() + 1); // remove b
      dir = ACperp;
      return false;
    }
  }

  // triangle contains the origin...
  // intersection occured
  return true;
}

bool
handle_simplex(std::vector<glm::vec3>& simplex, glm::vec3& dir)
{
  if (simplex.size() == 2)
    return line_case(simplex, dir);
  return triangle_case(simplex, dir);
};

// GJK algorithm
bool
gjk_squares_collide(const RotatedSquare& a, const RotatedSquare& b)
{
  // starting direction
  glm::vec3 dir = glm::normalize(b.center - a.center);

  // find the first point on simplex
  const auto minkow_point = support(a.points, b.points, dir);

  std::vector<glm::vec3> simplex{ minkow_point };

  // next direction is towards the origin
  dir = glm::vec3{ 0, 0, 0 } - simplex[0];

  while (true) {
    const auto A = support(a.points, b.points, dir);

    const float dprod = glm::dot(A, dir);
    if (dprod < 0)
      return false; // break: point did not pass the origin

    simplex.push_back(A);

    if (handle_simplex(simplex, dir))
      return true; // break: found collision
  }
};

// bool
// point_in_rotated_rectangle(const RotatedSquare& square, glm::vec2 point)
// {
//   // const auto& w = square.unrotated_w;
//   // const auto& h = square.unrotated_h;

//   // // center the point and rectangle to the origin
//   // const glm::vec2 tl = square.tl - square.center;
//   // const glm::vec2 tr = square.tr - square.center;
//   // const glm::vec2 br = square.br - square.center;
//   // const glm::vec2 bl = square.bl - square.center;
//   // const glm::vec2 t = point - square.center;

//   // // rotate the point around the origin
//   // const glm::ivec2 r = { t.x * std::cos(-square.theta) - t.y * std::sin(-square.theta),
//   //                        t.x * std::sin(-square.theta) + t.y * std::cos(-square.theta) };

//   // return r.x >= -w / 2.0f && r.x <= w / 2.0f && r.y >= -h / 2.0f && r.y <= h / 2.0f;
//   return false;
// };

RotatedSquare
transform_to_rotated_square(const TransformComponent& t)
{
  const auto& w = t.scale.x;
  const auto& h = t.scale.y;
  const auto& theta = t.rotation_radians.z;
  // const auto degrees = glm::degrees(theta);
  const glm::vec3 a = engine::rotate_point({ -w / 2.0f, h / 2.0f, 0.0f }, theta);
  const glm::vec3 b = engine::rotate_point({ w / 2.0f, h / 2.0f, 0.0f }, theta);
  const glm::vec3 c = engine::rotate_point({ w / 2.0f, -h / 2.0f, 0.0f }, theta);
  const glm::vec3 d = engine::rotate_point({ -w / 2.0f, -h / 2.0f, 0.0f }, theta);

  RotatedSquare square;
  square.theta = theta;
  square.center = t.position;
  square.unrotated_w = w;
  square.unrotated_h = h;
  square.points = {
    { a + square.center },
    { b + square.center },
    { c + square.center },
    { d + square.center },
  };
  return square;
};

} // namespace game2d