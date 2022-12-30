#include "colour/colour.hpp"

#include <math.h>

namespace engine {

// This function really should be constexpr
// cmath pow isnt constexpr
float
SRGBFloatToLinearFloat(const float f)
{
  if (f <= 0.04045f)
    return f / 12.92f;
  return pow((f + 0.055f) / 1.055f, 2.4f);
};

// This function really should be constexpr
// cmath pow isnt constexpr
float
LinearFloatToSRGBFloat(const float f)
{
  if (f <= 0.0031308f)
    return 12.92f * f;
  return 1.055f * pow(f, 1.0f / 2.4f) - 0.055f;
};

LinearColour
SRGBToLinear(const SRGBColour& colour)
{
  LinearColour output;
  output.r = SRGBFloatToLinearFloat(colour.r / 255.0f);
  output.g = SRGBFloatToLinearFloat(colour.g / 255.0f);
  output.b = SRGBFloatToLinearFloat(colour.b / 255.0f);
  output.a = colour.a;
  return output;
};

SRGBColour
LinearToSRGB(const LinearColour& colour)
{
  SRGBColour output;
  float r = LinearFloatToSRGBFloat(colour.r);
  float g = LinearFloatToSRGBFloat(colour.g);
  float b = LinearFloatToSRGBFloat(colour.b);
  output.r = static_cast<int>(r * 255.0f);
  output.g = static_cast<int>(g * 255.0f);
  output.b = static_cast<int>(b * 255.0f);
  output.a = colour.a;
  return output;
};

} // namespace engine
