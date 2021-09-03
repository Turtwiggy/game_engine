// header
#include "spritemap.hpp"

// game headers
#include "constants.hpp"

namespace game2d {

std::array<ImVec2, 2>
convert_sprite_to_uv(sprite::type type)
{
  const int pixels_x = PIXELS_ON_SHEET;
  const int pixels_y = PIXELS_ON_SHEET;

  // these are for the full texture
  // ImVec2 tl = ImVec2(0.0f, 0.0f);
  // ImVec2 br = ImVec2(1.0f, 1.0f);

  // this is for part of the texture
  auto offset = sprite::spritemap::get_sprite_offset(type);
  ImVec2 tl =
    ImVec2(((offset.x * pixels_x + 0.0f) / spritesheet_width), ((offset.y * pixels_y + 0.0f) / spritesheet_height));
  ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / spritesheet_width),
                     ((offset.y * pixels_y + pixels_x) / spritesheet_height));

  std::array<ImVec2, 2> coords = { tl, br };
  return coords;
};

}