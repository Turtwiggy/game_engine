#include "pch.hpp"

// header
#include "engine/maths/maths.hpp"

namespace engine {

float
deg2rad(const float& deg)
{
  return (deg * engine::PI) / 180.0f;
};

glm::vec2
angle_radians_to_direction(const float angle)
{
  return glm::vec2(glm::cos(angle), glm::sin(angle));
};

// puts an angle in the range [0, 2π]
float
clamp_axis(float angle)
{
  // range: [-2PI, 2PI]
  angle = std::fmod(angle, engine::TWO_PI);

  // range: [0, 2PI]
  if (angle < 0.0f)
    angle += engine::TWO_PI;

  return angle;
};

// puts an angle in the range [-π, π]
float
normalize_axis(float angle)
{
  angle = std::fmod(angle, engine::TWO_PI);
  if (angle > engine::PI)
    angle -= engine::TWO_PI;
  else if (angle < -engine::PI)
    angle += engine::TWO_PI;
  return angle;
};

float
clamp_angle(float rad_a, float rad_min, float rad_max)
{
  const float max_delta = clamp_axis(rad_max - rad_min) * 0.5;      // 0..π
  const float midpoint = clamp_axis(rad_min + max_delta);           // 0..2π
  const float delta_from_center = normalize_axis(rad_a - midpoint); // -π..π

  if (delta_from_center > max_delta)
    return normalize_axis(midpoint + max_delta);

  if (delta_from_center < -max_delta)
    return normalize_axis(midpoint - max_delta);

  return normalize_axis(rad_a);
};

// returns the angle from (WHICH) axis?
float
dir_to_angle_radians(const glm::vec2& dir)
{
  // note: -y for clockwise rotation
  float angle = atan2(dir.y, dir.x);

  return clamp_axis(angle);
};

float
distance_squared(const glm::vec3& i)
{
  return i.x * i.x + i.y * i.y + i.z * i.z;
};

glm::vec3
rand_unit_vector(RandomState& rnd)
{
  const auto a = rand_det_s(rnd.rng, 0.0f, 2.0f * PI);
  const auto z = rand_det_s(rnd.rng, -1.0f, 1.0f);
  const auto r = glm::sqrt(1 - z * z);
  return glm::vec3(r * glm::cos(a), r * glm::sin(a), z);
};

glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal)
{
  glm::vec3 in_unit_sphere = rand_unit_vector(rnd);
  if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
    return in_unit_sphere;
  else
    return -in_unit_sphere;
};

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
};

glm::vec2
rotate_point(const glm::vec2& point, const float angle_radians)
{
  const float cos_angle = std::cos(angle_radians);
  const float sin_angle = std::sin(angle_radians);

  return {
    point.x * cos_angle - point.y * sin_angle,
    point.x * sin_angle + point.y * cos_angle,
  };
};

int
zigzag_decode(uint32_t x)
{
  return (x >> 1) ^ (-(x & 1));
};

uint32_t
zigzag_encode(int x)
{
  return (x << 1) ^ (x >> (sizeof(int) * 8 - 1));
};

uint64_t
encode_cantor_pairing_function(int x_in, int y_in)
{
  uint32_t x = zigzag_encode(x_in);
  uint32_t y = zigzag_encode(y_in);

  // If you don't want to make a distinction between the pairs (a, b) and (b, a),
  // then sort a and b before applying the pairing function.
  // if (y < x) {
  //   // Swap X and Y
  //   int temp = x;
  //   x = y;
  //   y = temp;
  // }

  if (x < 0 || y < 0) {
    SDL_Log("%s", std::format("encode cantor pairing function not implemented negative ints").c_str());
    exit(1); // crash
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
};

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

  x = zigzag_decode(x);
  y = zigzag_decode(y);
};

float
scale(const float x, const float min, const float max, const float a, const float b)
{
  return ((b - a) * (x - min)) / (max - min) + a;
};

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

int
wrap(int val, int max)
{
  const int min = 0;
  const int range = max - min + 1;
  return ((val - min) % range + range) % range + min;
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

glm::ivec2
round_to_nearest_axis(const glm::vec2& v)
{
  const float abs_x = std::abs(v.x);
  const float abs_y = std::abs(v.y);

  if (abs_x > abs_y)
    return glm::ivec2(v.x > 0 ? 1 : -1, 0);

  if (abs_y > abs_x)
    return glm::ivec2(0, v.y > 0 ? 1 : -1);

  // v.x == v.y, face right by default
  return glm::ivec2(1, 0);
};

float
angle_degrees_flip_y_axis(float angle_degrees)
{
  float flipped_angle = (float)std::fmod(360 - angle_degrees, 360);

  if (flipped_angle < 0.0f)
    flipped_angle += 360.0f;

  return flipped_angle;
};

int
get_system_time_for_seed()
{
  auto now = std::chrono::high_resolution_clock::now();
  long long seed = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
  return seed;
};

bool
circle_collision(const Circle& a, const Circle& b)
{
  const auto d = b.pos - a.pos;
  const auto d2 = d.x * d.x + d.y * d.y;
  const float rad = a.radius + b.radius;
  const float rad_sqr = rad * rad;
  return d2 < rad_sqr;
};

} // namespace engine
