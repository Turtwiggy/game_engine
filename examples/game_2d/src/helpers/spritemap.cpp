// header
#include "spritemap.hpp"

// game headers
#include "constants.hpp"

namespace game2d {

std::array<ImVec2, 2>
convert_sprite_to_uv(sprite::type type, float pixels, glm::ivec2 wh)
{
  // these are for the full texture
  // ImVec2 tl = ImVec2(0.0f, 0.0f);
  // ImVec2 br = ImVec2(1.0f, 1.0f);

  // this is for part of the texture
  auto offset = sprite::spritemap::get_sprite_offset(type);
  ImVec2 tl = ImVec2(((offset.x * pixels + 0.0f) / wh.x), ((offset.y * pixels + 0.0f) / wh.y));
  ImVec2 br = ImVec2(((offset.x * pixels + pixels) / wh.x), ((offset.y * pixels + pixels) / wh.y));

  std::array<ImVec2, 2> coords = { tl, br };
  return coords;
};

}