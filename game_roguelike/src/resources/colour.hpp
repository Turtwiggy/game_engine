#pragma once

#include "colour/colour.hpp"

namespace game2d {

struct SINGLETON_ColoursComponent
{
  // https://colorhunt.co/palette/fff8ea9e7676815b5b594545
  const engine::SRGBColour pal_0_1 = { 255, 248, 234, 1.0f };
  const engine::SRGBColour pal_0_2 = { 158, 118, 118, 1.0f };
  const engine::SRGBColour pal_0_3 = { 129, 91, 91, 1.0f };
  const engine::SRGBColour pal_0_4 = { 89, 69, 69, 1.0f };
  const engine::LinearColour lin_pal_0_1 = engine::SRGBToLinear(pal_0_1);
  const engine::LinearColour lin_pal_0_2 = engine::SRGBToLinear(pal_0_2);
  const engine::LinearColour lin_pal_0_3 = engine::SRGBToLinear(pal_0_3);
  const engine::LinearColour lin_pal_0_4 = engine::SRGBToLinear(pal_0_4);

  // the game's live palette
  engine::SRGBColour primary = pal_0_1;
  engine::SRGBColour secondary = pal_0_2;
  engine::SRGBColour tertiary = pal_0_3;
  engine::SRGBColour quaternary = pal_0_4;
  engine::LinearColour lin_primary = engine::SRGBToLinear(primary);
  engine::LinearColour lin_secondary = engine::SRGBToLinear(secondary);
  engine::LinearColour lin_tertiary = engine::SRGBToLinear(tertiary);
  engine::LinearColour lin_quaternary = engine::SRGBToLinear(quaternary);

  const engine::SRGBColour background = pal_0_4;
  const engine::LinearColour lin_background = engine::SRGBToLinear(background);

  const engine::SRGBColour hit = { 172, 68, 37, 1.0f };
  const engine::LinearColour lin_hit = engine::SRGBToLinear(hit);

  const engine::SRGBColour white = { 255, 255, 255, 1.0f };
  const engine::LinearColour lin_white = engine::SRGBToLinear(white);
};

} // namespace game2d