#pragma once

#include "colour/colour.hpp"

#include <string>
#include <thread>

namespace game2d {

using srgb_ptr = std::shared_ptr<engine::SRGBColour>;
using lin_ptr = std::shared_ptr<engine::LinearColour>;

constexpr engine::SRGBColour
convert_hex_to_rgb(const int hex)
{
  return engine::SRGBColour(static_cast<int>(((hex >> 16) & 0xFF)), // r
                            static_cast<int>(((hex >> 8) & 0xFF)),  // g
                            static_cast<int>((hex & 0xFF)),         // b
                            1.0f);                                  // a
}

struct SINGLETON_ColoursComponent
{
  // https://colorhunt.co/palette/fff8ea9e7676815b5b594545
  const engine::SRGBColour pal_0_1 = { 255, 248, 234, 1.0f }; // lightest
  const engine::SRGBColour pal_0_2 = { 158, 118, 118, 1.0f };
  const engine::SRGBColour pal_0_3 = { 129, 91, 91, 1.0f };
  const engine::SRGBColour pal_0_4 = { 89, 69, 69, 1.0f }; // darkest

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