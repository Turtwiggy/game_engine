#pragma once

// other library headers
#include <glm/glm.hpp>

// c++ standard lib headers
#include <random>

namespace engine {

constexpr float PI{ 3.1415926535897932385f };
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

[[nodiscard]] constexpr float
distance_squared(const glm::vec3& i);

[[nodiscard]] glm::vec3
rand_unit_vector(RandomState& rnd);

[[nodiscard]] glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal);

[[nodiscard]] glm::vec3
rotate_point(const glm::vec3& point, const float angle_radians);

[[nodiscard]] uint64_t
encode_cantor_pairing_function(int x, int y);

void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y);

// scale x from [min,max] to [a,b]
[[nodiscard]] float
scale(const float x, const float min, const float max, const float a, const float b);

[[nodiscard]] glm::vec2
lerp_a_to_b_clamped_between_0_and_1(const glm::vec2 a, const glm::vec2 b, float t);

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

} // namespace engine
