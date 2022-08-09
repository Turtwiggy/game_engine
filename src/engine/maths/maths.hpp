#pragma once

// other library headers
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

// c++ standard lib headers
#include <random>

namespace engine {

constexpr float PI{ 3.1415926535897932385f };
constexpr float HALF_PI = PI / 2.0f;

//
// random
//

struct RandomState
{
  std::minstd_rand rng;
};

// https://github.com/20k/funvectormaths/blob/master/vec.hpp
/// falls into the pidgeonhole trap
/// and also std::minstd_rand isn't a very good rng
/// unfortunately the distributions in <random> are unspecified, so will not get
/// the same results across platforms, making them useless for anything which
/// needs reproducible random numbers, aka everything
template<typename T>
inline float
rand_det_s(T& rnd, float M, float MN)
{
  float scaled = (rnd() - rnd.min()) / (float)(rnd.max() - rnd.min() + 1.f);

  return scaled * (MN - M) + M;
}

[[nodiscard]] glm::vec2
angle_radians_to_direction(const float angle);

[[nodiscard]] float
dir_to_angle_radians(const glm::vec2& dir);

// glm::ivec2
// player::rotate_b_around_a(const GameObject2D& a, const GameObject2D& b, float radius, float angle)
// {
//   const glm::vec2 center{ 0.0f, 0.0f };
//   const glm::vec2 offset = glm::vec2(center.x + radius * cos(angle), center.y + radius * sin(angle));
//   // Position b in a circle around the a, and render the b in it's center.
//   const glm::vec2 pos = convert_top_left_to_centre(a);
//   const glm::vec2 rot_pos = pos + offset - (glm::vec2(b.physics_size) / 2.0f);
//   return glm::ivec2(int(rot_pos.x), int(rot_pos.y));
// };

[[nodiscard]] constexpr float
length_squared(const glm::vec3& i);

[[nodiscard]] glm::vec3
rand_unit_vector(RandomState& rnd);

[[nodiscard]] glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal);

[[nodiscard]] uint64_t
encode_cantor_pairing_function(int x, int y);

void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y);

// scale x from [min,max] to [a,b]
[[nodiscard]] constexpr float
scale(float x, float min, float max, float a, float b);

[[nodiscard]] glm::vec2
lerp_a_to_b_clamped_between_0_and_1(glm::vec2 a, glm::vec2 b, float t);

[[nodiscard]] glm::vec2
quadratic_curve(glm::vec2 a, glm::vec2 b, glm::vec2 c, float t);

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
}

} // namespace engine
