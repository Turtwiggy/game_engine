#pragma once

#include <nlohmann/json.hpp>

namespace engine {

// https://en.wikipedia.org/wiki/SRGB
struct SRGBColour
{
  int r = 0;
  int g = 0;
  int b = 0;
  int a = 0;

  constexpr SRGBColour() = default;

  // assumes converting 0-1 to 0-255
  constexpr SRGBColour(float cr, float cg, float cb, float ca)
    : r(static_cast<int>(cr * 255.0f))
    , g(static_cast<int>(cg * 255.0f))
    , b(static_cast<int>(cb * 255.0f))
    , a(static_cast<int>(ca * 255.0f)) {};

  // assumes 0-255 as input
  constexpr SRGBColour(int cr, int cg, int cb, int ca)
    : r(cr)
    , g(cg)
    , b(cb)
    , a(ca) {};
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