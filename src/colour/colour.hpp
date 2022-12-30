#pragma once

namespace engine {

// https://en.wikipedia.org/wiki/SRGB
struct SRGBColour
{
  int r = 0;
  int g = 0;
  int b = 0;
  float a = 0.0f;

  SRGBColour() = default;

  SRGBColour(float r, float g, float b, float a)
    : r(static_cast<int>(r * 255.0f))
    , g(static_cast<int>(g * 255.0f))
    , b(static_cast<int>(b * 255.0f))
    , a(a)
  {
  }

  SRGBColour(int r, int g, int b, float a)
    : r(r)
    , g(g)
    , b(b)
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