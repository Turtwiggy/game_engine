#include "util.hpp"

namespace game2d {

std::array<ImVec2, 2>
convert_sprite_to_uv(sprite::type type)
{
  const int size_x = 768;
  const int size_y = 352;
  const int cols_x = 48;
  const int cols_y = 22;
  const int pixels_x = size_x / cols_x;
  const int pixels_y = size_y / cols_y;

  // these are for the full texture
  // ImVec2 tl = ImVec2(0.0f, 0.0f);
  // ImVec2 br = ImVec2(1.0f, 1.0f);

  // this is for part of the texture
  auto offset = sprite::spritemap::get_sprite_offset(type);
  // clang-format off
  ImVec2 tl = ImVec2(((offset.x * pixels_x + 0.0f    ) / size_x), ((offset.y * pixels_y + 0.0f    ) / size_y));
  ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / size_x), ((offset.y * pixels_y + pixels_x) / size_y));
  // clang-format on

  std::array<ImVec2, 2> coords = { tl, br };
  return coords;
}

} // namespace game2d