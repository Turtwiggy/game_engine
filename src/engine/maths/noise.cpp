#include "pch.hpp"

#include "noise.hpp"

#define STB_PERLIN_IMPLEMENTATION
#include <stb_perlin.h>

namespace game2d {

float
perlin_noise_2d(float x, float y, int seed)
{
  // The "wrap" parameters can be used to create wraparound noise that
  // wraps at powers of two. The numbers MUST be powers of two. Specify
  // 0 to mean "don't care". (The noise always wraps every 256 due
  // details of the implementation, even if you ask for larger or no
  // wrapping.)

  return stb_perlin_noise3_seed(x, y, 0.0f, 0.0f, 0.0f, 0.0f, seed);
};

} // namespace game2d