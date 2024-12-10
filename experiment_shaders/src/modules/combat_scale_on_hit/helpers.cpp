#include "helpers.hpp"

#include "engine/maths/maths.hpp"

namespace game2d {

float
flip(float x)
{
  return 1.0f - x;
};

float
ease_in(float t)
{
  return t * t;
};

float
ease_out(float t)
{
  return flip(flip(t) * flip(t)); // not sure this is correct
};

float
ease_in_out(float t)
{
  return engine::lerp(ease_in(t), ease_out(t), t);
}

float
spike(float t)
{
  if (t <= .5f)
    return ease_in(t / 0.5f);

  return ease_in(flip(t) / 0.5f);
};

} // namespace game2d