#pragma once

// c++ standard lib headers
#include <random>

// other library headers
#include <glm/glm.hpp>

namespace fightingengine {

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

[[nodiscard]] float
length_squared(const glm::vec3& i);

[[nodiscard]] glm::vec3
rand_unit_vector(RandomState& rnd);

[[nodiscard]] glm::vec3
random_in_hemisphere(RandomState& rnd, glm::vec3& normal);

[[nodiscard]] uint64_t
encode_cantor_pairing_function(int x, int y);

[[nodiscard]] void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y);

// scale x from [min,max] to [a,b]
float
scale(float x, float min, float max, float a, float b);

} // namespace fightingengine
