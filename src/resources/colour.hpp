#pragma once

#include "engine/colour.hpp"

namespace game2d {

struct SINGLETON_ColoursComponent
{
  // palette
  const engine::SRGBColour red = { 232, 80, 100, 0.5f };
  const engine::SRGBColour desat_red = { 255, 121, 121, 1.0f };
  const engine::SRGBColour backdrop_red = { 232, 80, 100, 0.1f };
  const engine::SRGBColour cyan = { 8, 177, 190, 1.0f };
  const engine::SRGBColour dblue = { 49, 99, 188, 1.0f };
  const engine::SRGBColour white = { 255, 255, 255, 1.0f };
  const engine::SRGBColour feint_white = { 0.7f, 0.7f, 0.7f, 1.0f };
  const engine::SRGBColour green = { 100, 188, 49, 1.0f };
  const engine::SRGBColour grey = { 82, 88, 98, 1.0f };
  const engine::SRGBColour black = { 0, 0, 0, 1.0f };
  const engine::SRGBColour lightyellow = { 255, 255, 224, 1.0f };

  // game colours
  const engine::SRGBColour background = black;
  const engine::SRGBColour player_unit = dblue;
  const engine::SRGBColour enemy_unit = red;
  const engine::SRGBColour asteroid = red;
  const engine::SRGBColour bullet = lightyellow;
  const engine::SRGBColour turret = cyan;
};

} // namespace game2d