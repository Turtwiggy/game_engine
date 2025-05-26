#pragma once

// other library headers
#include <glm/fwd.hpp>

// c++ standard lib headers
#include <random>

namespace engine {

constexpr float PI{ 3.1415926535897932385f };
constexpr float TWO_PI = 2.0f * PI;
constexpr float HALF_PI = PI / 2.0f;
constexpr float Deg2Rad = PI / 180.0f;
constexpr float Rad2Deg = 180.0f / PI;

//
// random
//

struct RandomState
{
  std::minstd_rand rng;

  RandomState() = default;
  RandomState(const int seed) { rng.seed(seed); }
};

// https://github.com/20k/funvectormaths/blob/master/vec.hpp
/// falls into the pidgeonhole trap
/// and also std::minstd_rand isn't a very good rng
/// unfortunately the distributions in <random> are unspecified, so will not get
/// the same results across platforms, making them useless for anything which
/// needs reproducible random numbers, aka everything

template<typename T>
inline float
rand_det_s(T& rnd, const float M, const float MN)
{
  const float scaled = (rnd() - rnd.min()) / (float)(rnd.max() - rnd.min() + 1.f);

  return scaled * (MN - M) + M;
};

// note: non inclusive, so e.g. M=0, MN=4, produces [0, 1, 2, 3]
template<typename T>
inline int
rand_det_s(T& rnd, const int M, const int MN)
{
  const float scaled = (rnd() - rnd.min()) / (float)(rnd.max() - rnd.min() + 1.f);

  return static_cast<int>(scaled * (MN - M) + M);
};

template<typename T>
inline float
rand_01(T& rnd)
{
  return rand_det_s(rnd, 0.0f, 1.0f);
};

[[nodiscard]] float
deg2rad(const float& deg);

[[nodiscard]] glm::vec2
angle_radians_to_direction(const float angle);

[[nodiscard]] float
dir_to_angle_radians(const glm::vec2& dir);

// puts an angle in the range [0, 2π]
[[nodiscard]] float
clamp_axis(float angle);

// puts an angle in the range [-π, π]
[[nodiscard]] float
normalize_axis(float angle);

[[nodiscard]] float
clamp_angle(float rad_a, float rad_min, float rad_max);

[[nodiscard]] float
distance_squared(const glm::vec3& i);

[[nodiscard]] glm::vec3
rand_unit_vector(RandomState& rnd);

[[nodiscard]] glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal);

[[nodiscard]] glm::vec3
rotate_point(const glm::vec3& point, const float angle_radians);

[[nodiscard]] glm::vec2
rotate_point(const glm::vec2& point, const float angle_radians);

[[nodiscard]] uint64_t
encode_cantor_pairing_function(int x, int y);

void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y);

// scale x from [min,max] to [a,b]
[[nodiscard]] float
scale(const float x, const float min, const float max, const float a, const float b);

[[nodiscard]] float
lerp(const float a, const float b, const float t);

[[nodiscard]] glm::vec2
lerp_a_to_b_clamped_between_0_and_1(const glm::vec2 a, const glm::vec2 b, float t);

// wraps between [0, max]
[[nodiscard]]
int
wrap(int val, int max);

[[nodiscard]] glm::vec2
quadratic_curve(const glm::vec2 a, const glm::vec2 b, const glm::vec2 c, float t);

// any line can be written in parametric form as:
// point + direction * t
struct Ray
{
  glm::vec3 origin;
  glm::vec3 dir;
};

[[nodiscard]] inline glm::vec3
ray_at(const Ray r, float t)
{
  return r.origin + (t * r.dir);
};

[[nodiscard]] glm::vec2
normalize_safe(const glm::vec2& value);
[[nodiscard]] glm::vec3
normalize_safe(const glm::vec3& value);

glm::ivec2
round_to_nearest_axis(const glm::vec2& v);

float
angle_degrees_flip_y_axis(float angle_degrees);

[[nodiscard]] int
get_system_time_for_seed();

struct Circle
{
  glm::vec2 pos;
  float radius;
};

bool
circle_collision(const Circle& a, const Circle& b);

} // namespace engine
