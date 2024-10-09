#pragma once

#include <nlohmann/json.hpp>

namespace engine {

// https://en.wikipedia.org/wiki/SRGB
struct SRGBColour
{
  int r = 0;
  int g = 0;
  int b = 0;
  float a = 0.0f;

  constexpr SRGBColour() = default;

  // assumes converting 0-1 to 0-255
  constexpr SRGBColour(float r, float g, float b, float a)
    : r(static_cast<int>(r * 255.0f))
    , g(static_cast<int>(g * 255.0f))
    , b(static_cast<int>(b * 255.0f))
    , a(a)
  {
  }

  // assumes 0-255 as input (except alpha, 0-1)
  constexpr SRGBColour(int r, int g, int b, float a)
    : r(r)
    , g(g)
    , b(b)
    , a(a) {};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SRGBColour, r, g, b);
};

struct LinearColour
{
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 0.0f;

  constexpr LinearColour() = default;

  constexpr LinearColour(float cr, float cg, float cb, float ca)
    : r(cr)
    , g(cg)
    , b(cb)
    , a(ca)
  {
  }

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(LinearColour, r, g, b);
};

LinearColour
SRGBToLinear(const SRGBColour& colour);

SRGBColour
LinearToSRGB(const LinearColour& colour);

} // namespace engine