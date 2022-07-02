#pragma once

#include <cmath>
#include <cstdint>

namespace engine {

// https://en.wikipedia.org/wiki/SRGB
struct SRGBColour
{
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 0.0f;

  SRGBColour() = default;

  SRGBColour(float r, float g, float b, float a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
  {
  }

  SRGBColour(int r, int g, int b, float a)
    : r(r / 255.0f)
    , g(g / 255.0f)
    , b(b / 255.0f)
    , a(a){};
};

struct LinearColour
{
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 0.0f;

  LinearColour() = default;
};

LinearColour
SRGBToLinear(const SRGBColour& colour);

SRGBColour
LinearToSRGB(const LinearColour& colour);

} // namespace engine