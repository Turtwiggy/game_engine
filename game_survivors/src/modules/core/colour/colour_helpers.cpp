#include "pch.hpp"

#include "colour_helpers.hpp"

#include "engine/maths/maths.hpp"

namespace game2d {

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

engine::SRGBColour
lerp_colour(engine::SRGBColour a, engine::SRGBColour b, float percent)
{
  float a_r = SRGBFloatToLinearFloat(a.r / 255.0f);
  float a_g = SRGBFloatToLinearFloat(a.g / 255.0f);
  float a_b = SRGBFloatToLinearFloat(a.b / 255.0f);

  float b_r = SRGBFloatToLinearFloat(b.r / 255.0f);
  float b_g = SRGBFloatToLinearFloat(b.g / 255.0f);
  float b_b = SRGBFloatToLinearFloat(b.b / 255.0f);

  float mix_r = engine::lerp(a_r, b_r, percent);
  float mix_g = engine::lerp(a_g, b_g, percent);
  float mix_b = engine::lerp(a_b, b_b, percent);

  const int srgb_r = (int)(LinearFloatToSRGBFloat(mix_r) * 255.0f);
  const int srgb_g = (int)(LinearFloatToSRGBFloat(mix_g) * 255.0f);
  const int srgb_b = (int)(LinearFloatToSRGBFloat(mix_b) * 255.0f);
  const int srgb_a = (int)((a.a + b.a) * 0.5f) * 255.0f;
  return engine::SRGBColour{ srgb_r, srgb_g, srgb_b, srgb_a };
}

} // namespace game2d