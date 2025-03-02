#include "ui_colours_helpers.hpp"

namespace game2d {

engine::SRGBColour
hex_to_srgb(const std::string& hex)
{
  const std::string hex_without_hash = hex.substr(hex.find("#") + 1, hex.length());
  const std::string str_r = hex_without_hash.substr(0, 2);
  const std::string str_g = hex_without_hash.substr(2, 2);
  const std::string str_b = hex_without_hash.substr(4, 2);
  const int r = std::stoi(str_r, 0, 16);
  const int g = std::stoi(str_g, 0, 16);
  const int b = std::stoi(str_b, 0, 16);
  return { r, g, b, 255 };
};

ImVec4
convert_my_to_im_vec(const engine::SRGBColour& col)
{
  return ImVec4{ col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a / 255.0f };
};

ImU32
convert_my_to_im(const engine::SRGBColour& col)
{
  return IM_COL32(col.r, col.g, col.b, col.a);
};

} // namespace game2d