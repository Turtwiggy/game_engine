#pragma once

#include "colour/colour.hpp"

#include <thread>

namespace game2d {

using srgb_ptr = std::shared_ptr<engine::SRGBColour>;
using lin_ptr = std::shared_ptr<engine::LinearColour>;

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

  const engine::SRGBColour pal_background = pal_0_4;
  const engine::SRGBColour pal_hit = { 172, 68, 37, 1.0f };
  const engine::SRGBColour pal_white = { 255, 255, 255, 1.0f };
  const engine::LinearColour lin_pal_hit = engine::SRGBToLinear(pal_hit);
  const engine::LinearColour lin_pal_white = engine::SRGBToLinear(pal_white);
  const engine::LinearColour lin_pal_background = engine::SRGBToLinear(pal_background);

  // the game's live palette
  srgb_ptr primary = std::make_shared<engine::SRGBColour>(pal_0_1);
  srgb_ptr secondary = std::make_shared<engine::SRGBColour>(pal_0_2);
  srgb_ptr tertiary = std::make_shared<engine::SRGBColour>(pal_0_3);
  srgb_ptr quaternary = std::make_shared<engine::SRGBColour>(pal_0_4);
  srgb_ptr background = std::make_shared<engine::SRGBColour>(pal_background);
  srgb_ptr hit = std::make_shared<engine::SRGBColour>(pal_hit);
  srgb_ptr white = std::make_shared<engine::SRGBColour>(pal_white);
  lin_ptr lin_primary = std::make_shared<engine::LinearColour>(lin_pal_0_1);
  lin_ptr lin_secondary = std::make_shared<engine::LinearColour>(lin_pal_0_2);
  lin_ptr lin_tertiary = std::make_shared<engine::LinearColour>(lin_pal_0_3);
  lin_ptr lin_quaternary = std::make_shared<engine::LinearColour>(lin_pal_0_4);
  lin_ptr lin_background = std::make_shared<engine::LinearColour>(lin_pal_background);
  lin_ptr lin_hit = std::make_shared<engine::LinearColour>(lin_pal_hit);
  lin_ptr lin_white = std::make_shared<engine::LinearColour>(lin_pal_white);
};

} // namespace game2d