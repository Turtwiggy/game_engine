
// header
#include "engine/maths/maths.hpp"

#include <SDL2/SDL_log.h>
#include <format>

namespace engine {

float
deg2rad(const float& deg)
{
  return (deg * engine::PI) / 180.0f;
}

glm::vec2
angle_radians_to_direction(const float angle)
{
  return glm::vec2(glm::cos(angle), glm::sin(angle));
}

// returns the angle from (WHICH) axis?
float
dir_to_angle_radians(const glm::vec2& dir)
{
  return atan2(dir.y, dir.x) + engine::PI;
}

constexpr float
distance_squared(const glm::vec3& i)
{
  return i.x * i.x + i.y * i.y + i.z * i.z;
}

glm::vec3
rand_unit_vector(RandomState& rnd)
{
  auto a = rand_det_s(rnd.rng, 0.0f, 2.0f * PI);
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

glm::vec3
rotate_point(const glm::vec3& point, const float angle_radians)
{
  const float cos_angle = std::cos(angle_radians);
  const float sin_angle = std::sin(angle_radians);
  return {
    point.x * cos_angle - point.y * sin_angle,
    point.x * sin_angle + point.y * cos_angle,
    0.0f,
  };
}

uint64_t
encode_cantor_pairing_function(int x, int y)
{
  // If you don't want to make a distinction between the pairs (a, b) and (b, a),
  // then sort a and b before applying the pairing function.

  if (x < 0 || y < 0) {
    SDL_Log("%s", std::format("encode cantor pairing function not implemented negative ints").c_str());
    exit(1); // crash
  }

  if (y < x) {
    // Swap X and Y
    int temp = x;
    x = y;
    y = temp;
  }

  int64_t p = 0;
  size_t i = 0;
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
  size_t i = 0;
  while (p) {
    x |= ((uint32_t)(p & 1) << i);
    p >>= 1;
    y |= ((uint32_t)(p & 1) << i);
    p >>= 1;
    i++;
  }
}

float
scale(const float x, const float min, const float max, const float a, const float b)
{
  return ((b - a) * (x - min)) / (max - min) + a;
}

float
lerp(const float a, const float b, const float t)
{
  return a + (b - a) * t;
};

glm::vec2
lerp_a_to_b_clamped_between_0_and_1(const glm::vec2 a, const glm::vec2 b, float t)
{
  const float aval = lerp(a.x, b.x, glm::clamp(t, 0.0f, 1.0f));
  const float bval = lerp(a.y, b.y, glm::clamp(t, 0.0f, 1.0f));
  return { aval, bval };
};

glm::vec2
quadratic_curve(const glm::vec2 a, const glm::vec2 b, const glm::vec2 c, const float t)
{
  const auto p0 = lerp_a_to_b_clamped_between_0_and_1(a, b, t);
  const auto p1 = lerp_a_to_b_clamped_between_0_and_1(b, c, t);
  return lerp_a_to_b_clamped_between_0_and_1(p0, p1, t);
}

glm::vec2
normalize_safe(const glm::vec2& value)
{
  const float EPSILON = 0.00001f;
  const auto& equal = [&EPSILON](const float a, const float b) { return glm::abs(a - b) < EPSILON; };
  if (equal(value.x, 0.0f) && equal(value.y, 0.0f))
    return { 0.0f, 0.0f };
  return glm::normalize(value);
};

glm::vec3
normalize_safe(const glm::vec3& value)
{
  const float EPSILON = 0.00001f;
  const auto& equal = [&EPSILON](const float a, const float b) { return glm::abs(a - b) < EPSILON; };
  if (equal(value.x, 0.0f) && equal(value.y, 0.0f) && equal(value.z, 0.0f))
    return { 0.0f, 0.0f, 0.0f };
  return glm::normalize(value);
};

} // namespace engine
