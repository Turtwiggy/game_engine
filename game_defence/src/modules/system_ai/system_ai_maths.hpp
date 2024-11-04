#pragma once

#include <cmath>

namespace game2d {

// https://en.wikipedia.org/wiki/Exponential_decay
inline float
ExponentialDecay(float x, float factor = -5)
{
  // hmm: (100 - distance ^ 3) / (100 ^ 3)
  const auto e = std::exp(1.0f);
  const auto y = std::pow(e, factor * x);
  return y;
}

// https://en.wikipedia.org/wiki/Generalised_logistic_function
// The sigmoid expects:
// input: 0 =< x <= 1
// output: 0 =< y <= 1
inline float
Logistic(float x)
{
  const auto e = std::exp(1.0f);
  const auto y = 1.0f / (1.0f + std::pow(e, (12 * x) - 5));
  return y;
}

// inline float Linear(float x_max, float x)
// {
//   return (x_max - x) / x_max;
// }

// inline float Binary()
// {
// }

// vertex at (h, k)
// x-axis roots at r1, r2
inline float
Parabola(float x, float r1, float r2, float h, float k)
{
  auto y = -k / ((r1 - h) * (r2 - h) * std::pow(x - h, 2) + k);
  return y;
}

} // namespace game2d