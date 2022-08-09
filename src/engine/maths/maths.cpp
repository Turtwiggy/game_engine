
// header
#include "engine/maths/maths.hpp"

namespace engine {

glm::vec2
angle_radians_to_direction(const float angle)
{
  return glm::vec2(glm::sin(angle), -glm::cos(angle));
}

float
dir_to_angle_radians(const glm::vec2& dir)
{
  return atan2(dir.y, dir.x) + engine::PI;
}

constexpr float
length_squared(const glm::vec3& i)
{
  return i.x * i.x + i.y * i.y + i.z * i.z;
}

glm::vec3
rand_unit_vector(RandomState& rnd)
{
  auto a = rand_det_s(rnd.rng, 0, 2.0f * PI);
  auto z = rand_det_s(rnd.rng, -1.0f, 1.0f);
  auto r = glm::sqrt(1 - z * z);
  return glm::vec3(r * glm::cos(a), r * glm::sin(a), z);
}

glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal)
{
  glm::vec3 in_unit_sphere = rand_unit_vector(rnd);
  if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
    return in_unit_sphere;
  else
    return -in_unit_sphere;
}

uint64_t
encode_cantor_pairing_function(int x, int y)
{
  // If you don't want to make a distinction between the pairs (a, b) and (b, a),
  // then sort a and b before applying the pairing function.
  if (y < x) {
    // Swap X and Y
    uint64_t temp = x;
    x = y;
    y = temp;
  }

  int64_t p = 0;
  int i = 0;
  while (x || y) {
    p |= ((uint64_t)(x & 1) << i);
    x >>= 1;
    p |= ((uint64_t)(y & 1) << (i + 1));
    y >>= 1;
    i += 2;
  }
  return p;
}

void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y)
{
  x = 0;
  y = 0;
  int i = 0;
  while (p) {
    x |= ((uint32_t)(p & 1) << i);
    p >>= 1;
    y |= ((uint32_t)(p & 1) << i);
    p >>= 1;
    i++;
  }
}

constexpr float
scale(const float x, const float min, const float max, const float a, const float b)
{
  return ((b - a) * (x - min)) / (max - min) + a;
}

glm::vec2
lerp_a_to_b_clamped_between_0_and_1(glm::vec2 a, glm::vec2 b, float t)
{
  return glm::lerp(a, b, glm::clamp(t, 0.0f, 1.0f));
}

glm::vec2
quadratic_curve(glm::vec2 a, glm::vec2 b, glm::vec2 c, float t)
{
  glm::vec2 p0 = lerp_a_to_b_clamped_between_0_and_1(a, b, t);
  glm::vec2 p1 = lerp_a_to_b_clamped_between_0_and_1(b, c, t);
  return lerp_a_to_b_clamped_between_0_and_1(p0, p1, t);
}

} // namespace engine
