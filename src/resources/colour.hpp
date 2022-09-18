#pragma once

#include "engine/colour.hpp"

namespace game2d {

struct SINGLETON_ColoursComponent
{
  // palette

  // r...
  const engine::SRGBColour red = { 232, 80, 100, 0.5f };
  const engine::SRGBColour desat_red = { 255, 121, 121, 1.0f };
  const engine::SRGBColour backdrop_red = { 232, 80, 100, 0.1f };
  // o...
  const engine::SRGBColour orange = { 255, 127, 80, 1.0f };
  // y...
  const engine::SRGBColour lightyellow = { 236, 204, 104, 1.0f };
  // g...
  const engine::SRGBColour green = { 100, 188, 49, 1.0f };
  // b...
  const engine::SRGBColour blue_0 = { 12, 28, 43, 1.0f }; // lighter
  const engine::SRGBColour blue_1 = { 11, 21, 32, 1.0f }; // darker
  const engine::SRGBColour cyan = { 8, 177, 190, 1.0f };
  const engine::SRGBColour dblue = { 49, 99, 188, 1.0f };
  // i...
  // v...
  // black...
  const engine::SRGBColour black = { 0, 0, 0, 1.0f };
  const engine::SRGBColour grey = { 82, 88, 98, 1.0f };
  // white...
  const engine::SRGBColour white = { 255, 255, 255, 1.0f };
  const engine::SRGBColour feint_white = { 0.7f, 0.7f, 0.7f, 0.2f };

  const engine::LinearColour lin_red = engine::SRGBToLinear(red);
  const engine::LinearColour lin_desat_red = engine::SRGBToLinear(desat_red);
  const engine::LinearColour lin_backdrop_red = engine::SRGBToLinear(backdrop_red);
  const engine::LinearColour lin_cyan = engine::SRGBToLinear(cyan);
  const engine::LinearColour lin_dblue = engine::SRGBToLinear(dblue);
  const engine::LinearColour lin_white = engine::SRGBToLinear(white);
  const engine::LinearColour lin_feint_white = engine::SRGBToLinear(feint_white);
  const engine::LinearColour lin_green = engine::SRGBToLinear(green);
  const engine::LinearColour lin_grey = engine::SRGBToLinear(grey);
  const engine::LinearColour lin_black = engine::SRGBToLinear(black);
  const engine::LinearColour lin_lightyellow = engine::SRGBToLinear(lightyellow);
  const engine::LinearColour lin_orange = engine::SRGBToLinear(orange);
  const engine::LinearColour lin_blue_0 = engine::SRGBToLinear(blue_0);
  const engine::LinearColour lin_blue_1 = engine::SRGBToLinear(blue_1);

  // game colours
  const engine::SRGBColour background = blue_0;
  const engine::SRGBColour player_unit = white;
  const engine::SRGBColour enemy_unit = red;
  const engine::SRGBColour wall = red;
  const engine::SRGBColour asteroid = red;
  const engine::SRGBColour bullet = lightyellow;
  const engine::SRGBColour turret = cyan;
  const engine::SRGBColour shield = cyan;
  const engine::SRGBColour floor = feint_white;

  // linear
  const engine::LinearColour lin_background = engine::SRGBToLinear(background);
  const engine::LinearColour lin_player_unit = engine::SRGBToLinear(player_unit);
  const engine::LinearColour lin_enemy_unit = engine::SRGBToLinear(enemy_unit);
  const engine::LinearColour lin_wall = engine::SRGBToLinear(wall);
  const engine::LinearColour lin_asteroid = engine::SRGBToLinear(asteroid);
  const engine::LinearColour lin_bullet = engine::SRGBToLinear(bullet);
  const engine::LinearColour lin_turret = engine::SRGBToLinear(turret);
  const engine::LinearColour lin_shield = engine::SRGBToLinear(shield);
  const engine::LinearColour lin_floor = engine::SRGBToLinear(floor);
};

} // namespace game2d