#include "colour.hpp"

#include <math.h>

namespace engine {

// This function really should be constexpr
// cmath pow isnt constexpr
float
SRGBFloatToLinearFloat(const float f)
{
  if (f <= 0.04045f)
    return f / 12.92f;
  return static_cast<float>(pow(f + (0.055f / 1.055f), 2.4f));
};

// This function really should be constexpr
// cmath pow isnt constexpr
float
LinearFloatToSRGBFloat(const float f)
{
  if (f <= 0.0031308f)
    return 12.92f * f;
  return static_cast<float>(1.055f * pow(f, 1.0f / 2.4f) - 0.055f);
};

LinearColour
SRGBToLinear(const SRGBColour& colour)
{
  LinearColour output;
  output.r = SRGBFloatToLinearFloat(colour.r);
  output.g = SRGBFloatToLinearFloat(colour.g);
  output.b = SRGBFloatToLinearFloat(colour.b);
  output.a = colour.a;
  return output;
};

SRGBColour
LinearToSRGB(const LinearColour& colour)
{
  SRGBColour output;
  output.r = LinearFloatToSRGBFloat(colour.r);
  output.g = LinearFloatToSRGBFloat(colour.g);
  output.b = LinearFloatToSRGBFloat(colour.b);
  output.a = colour.a;
  return output;
};

} // namespace engine
