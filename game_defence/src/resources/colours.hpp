#pragma once

#include "colour/colour.hpp"

#include <string>
#include <thread>

namespace game2d {

// engine::SRGBColour
// convert_hex_to_rgb(const int hex)
// {
//   return engine::SRGBColour(static_cast<int>(((hex >> 16) & 0xFF)), // r
//                             static_cast<int>(((hex >> 8) & 0xFF)),  // g
//                             static_cast<int>((hex & 0xFF)),         // b
//                             1.0f);                                  // a
// }

// https://colorhunt.co/palette/fff8ea9e7676815b5b594545
static const engine::SRGBColour pal_0{ 255, 248, 234, 1.0f }; // lightest
static const engine::SRGBColour pal_1{ 158, 118, 118, 1.0f };
static const engine::SRGBColour pal_2{ 129, 91, 91, 1.0f };
static const engine::SRGBColour pal_3{ 89, 69, 69, 1.0f }; // darkest
static const engine::LinearColour lin_pal_0 = engine::SRGBToLinear(pal_0);
static const engine::LinearColour lin_pal_1 = engine::SRGBToLinear(pal_1);
static const engine::LinearColour lin_pal_2 = engine::SRGBToLinear(pal_2);
static const engine::LinearColour lin_pal_3 = engine::SRGBToLinear(pal_3);
static const engine::SRGBColour pal_background = pal_3;
static const engine::SRGBColour pal_hit = { 172, 68, 37, 1.0f };
static const engine::SRGBColour pal_white = { 255, 255, 255, 1.0f };
static const engine::LinearColour lin_pal_hit = engine::SRGBToLinear(pal_hit);
static const engine::LinearColour lin_pal_white = engine::SRGBToLinear(pal_white);
static const engine::LinearColour lin_pal_background = engine::SRGBToLinear(pal_background);

static const engine::SRGBColour pal_hot_pink = { 251, 161, 183, 1.0f };
static const engine::SRGBColour pal_light_pink = { 255, 209, 218, 1.0f };
static const engine::SRGBColour pal_lighter_pink = { 255, 240, 245, 1.0f };
// static const engine::SRGBColour pal_yellow = { 255, 219, 170, 1.0f };
static const engine::LinearColour lin_pal_hot_pink = engine::SRGBToLinear(pal_hot_pink);
static const engine::LinearColour lin_pal_light_pink = engine::SRGBToLinear(pal_light_pink);
static const engine::LinearColour lin_pal_lighter_pink = engine::SRGBToLinear(pal_lighter_pink);
// static const engine::LinearColour lin_pal_yellow = engine::SRGBToLinear(pal_yellow);

// misc
static const engine::SRGBColour pal_blue = { 68, 119, 206, 1.0f };
static const engine::SRGBColour pal_orange = { 253, 141, 20, 1.0f };
static const engine::SRGBColour pal_yellow = { 255, 225, 123, 1.0f };
static const engine::LinearColour lin_pal_blue = engine::SRGBToLinear(pal_blue);
static const engine::LinearColour lin_pal_orange = engine::SRGBToLinear(pal_orange);
static const engine::LinearColour lin_pal_yellow = engine::SRGBToLinear(pal_yellow);

struct SINGLETON_ColoursComponent
{
  // Gonna have to develop somthing to cycle through these
  // https://colorhunt.co/palette/4a55a27895cba0bfe0c5dff8
  // https://colorhunt.co/palette/2d4356435b66a76f6feab2a0
  // https://colorhunt.co/palette/222831393e4600adb5eeeeee
  // https://colorhunt.co/palette/f9f7f7dbe2ef3f72af112d4e
  // https://colorhunt.co/palette/f4eeffdcd6f7a6b1e1424874
  // https://colorhunt.co/palette/1b262c0f4c753282b8bbe1fa
  // https://colorhunt.co/palette/2121213232320d737714ffec
  // https://colorhunt.co/palette/f0f5f9c9d6df52616b1e2022
  // https://colorhunt.co/palette/ffc8c8ff9999444f5a3e4149
  // https://colorhunt.co/palette/0000004e4feb068fffeeeeee
  // https://colorhunt.co/palette/164b601b6b934fc0d0a2ff86
  // https://colorhunt.co/palette/ff9b9bffd6a5fffec4cbffa9
  // https://colorhunt.co/palette/331d2c3f2e3ea78295efe1d1
  // https://colorhunt.co/palette/78c1f39be8d8e2f6caf8fdcf
  // https://colorhunt.co/palette/2d4059ea5455f07b3fffd460
  // https://colorhunt.co/palette/ffc8c8ff9999444f5a3e4149
  // https://colorhunt.co/palette/14285027496d0c7b9300a8cc
  // https://colorhunt.co/palette/edf1d69dc08b60996640513b
  // https://colorhunt.co/palette/a75d5dd3756bf0997dffc3a1
  // https://colorhunt.co/palette/00000052057b892cdcbc6ff1
  // https://colorhunt.co/palette/7579e79ab3f5a3d8f4b9fffc

  // the game's live palette
  std::shared_ptr<engine::SRGBColour> primary = std::make_shared<engine::SRGBColour>(pal_0);
  std::shared_ptr<engine::SRGBColour> secondary = std::make_shared<engine::SRGBColour>(pal_1);
  std::shared_ptr<engine::SRGBColour> tertiary = std::make_shared<engine::SRGBColour>(pal_2);
  std::shared_ptr<engine::SRGBColour> quaternary = std::make_shared<engine::SRGBColour>(pal_3);
  std::shared_ptr<engine::SRGBColour> background = std::make_shared<engine::SRGBColour>(pal_background);
  std::shared_ptr<engine::SRGBColour> hit = std::make_shared<engine::SRGBColour>(pal_hit);
  std::shared_ptr<engine::SRGBColour> white = std::make_shared<engine::SRGBColour>(pal_white);
  std::shared_ptr<engine::LinearColour> lin_primary = std::make_shared<engine::LinearColour>(lin_pal_0);
  std::shared_ptr<engine::LinearColour> lin_secondary = std::make_shared<engine::LinearColour>(lin_pal_1);
  std::shared_ptr<engine::LinearColour> lin_tertiary = std::make_shared<engine::LinearColour>(lin_pal_2);
  std::shared_ptr<engine::LinearColour> lin_quaternary = std::make_shared<engine::LinearColour>(lin_pal_3);
  std::shared_ptr<engine::LinearColour> lin_background = std::make_shared<engine::LinearColour>(lin_pal_background);
  std::shared_ptr<engine::LinearColour> lin_hit = std::make_shared<engine::LinearColour>(lin_pal_hit);
  std::shared_ptr<engine::LinearColour> lin_white = std::make_shared<engine::LinearColour>(lin_pal_white);

  // another palette
  std::shared_ptr<engine::SRGBColour> hot_pink = std::make_shared<engine::SRGBColour>(pal_hot_pink);
  std::shared_ptr<engine::SRGBColour> light_pink = std::make_shared<engine::SRGBColour>(pal_light_pink);
  std::shared_ptr<engine::SRGBColour> lighter_pink = std::make_shared<engine::SRGBColour>(pal_lighter_pink);
  // std::shared_ptr<engine::SRGBColour> yellow = std::make_shared<engine::SRGBColour>(pal_yellow);
  std::shared_ptr<engine::LinearColour> lin_hot_pink = std::make_shared<engine::LinearColour>(lin_pal_hot_pink);
  std::shared_ptr<engine::LinearColour> lin_light_pink = std::make_shared<engine::LinearColour>(lin_pal_light_pink);
  std::shared_ptr<engine::LinearColour> lin_lighter_pink = std::make_shared<engine::LinearColour>(lin_pal_lighter_pink);
  // std::shared_ptr<engine::LinearColour> lin_yellow = std::make_shared<engine::LinearColour>(lin_pal_yellow);

  // misc colours
  std::shared_ptr<engine::SRGBColour> blue = std::make_shared<engine::SRGBColour>(pal_blue);
  std::shared_ptr<engine::SRGBColour> orange = std::make_shared<engine::SRGBColour>(pal_orange);
  std::shared_ptr<engine::SRGBColour> yellow = std::make_shared<engine::SRGBColour>(pal_yellow);
  std::shared_ptr<engine::LinearColour> lin_blue = std::make_shared<engine::LinearColour>(lin_pal_blue);
  std::shared_ptr<engine::LinearColour> lin_orange = std::make_shared<engine::LinearColour>(lin_pal_orange);
  std::shared_ptr<engine::LinearColour> lin_yellow = std::make_shared<engine::LinearColour>(lin_pal_yellow);
};

} // namespace game2d