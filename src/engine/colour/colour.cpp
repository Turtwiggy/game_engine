#include "engine/colour/colour.hpp"

#include <cmath>

namespace engine {

// This function really should be constexpr but math pow isnt constexpr
constexpr float
SRGBFloatToLinearFloat(const float f)
{
  if (f <= 0.04045f)
    return f / 12.92f;
  return std::pow((f + 0.055f) / 1.055f, 2.4f);
};

// This function really should be constexpr but math pow isnt constexpr
constexpr float
LinearFloatToSRGBFloat(const float f)
{
  if (f <= 0.0031308f)
    return 12.92f * f;
  return 1.055f * std::pow(f, 1.0f / 2.4f) - 0.055f;
};

LinearColour
SRGBToLinear(const SRGBColour& colour)
{
  float r = SRGBFloatToLinearFloat(colour.r / 255.0f);
  float g = SRGBFloatToLinearFloat(colour.g / 255.0f);
  float b = SRGBFloatToLinearFloat(colour.b / 255.0f);
  float a = colour.a / 255.0f;
  return LinearColour(r, g, b, a);
};

SRGBColour
LinearToSRGB(const LinearColour& colour)
{
  float r = LinearFloatToSRGBFloat(colour.r);
  float g = LinearFloatToSRGBFloat(colour.g);
  float b = LinearFloatToSRGBFloat(colour.b);
  float a = colour.a * 255.0f;
  return SRGBColour(r, g, b, a);
};

} // namespace engine
