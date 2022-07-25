#pragma once

#include <string>

namespace game2d {

struct SINGLETON_Textures
{
  // TODO: texture unit (slot on gpu)
  // this is currently tied to
  // glActiveTexture() in render's system.cpp
  // this should not be like this
  const int tex_unit_kenny = 0;
  const int tex_unit_custom = 1;
  const int tex_unit_sprout = 2;
  const int tex_unit_logo = 3;
  const int tex_unit_map = 4;
  const int tex_unit_linear_main_scene = 5;
  const int tex_unit_linear_lighting = 6;
  const int tex_unit_srgb_main_scene = 7;

  // texture ids (id after bound)
  int tex_id_kenny = 0;
  int tex_id_custom = 0;
  int tex_id_sprout = 0;
  int tex_id_logo = 0;
  int tex_id_map = 0;
  int tex_id_linear_main_scene = 0;
  int tex_id_linear_lighting = 0;
  int tex_id_srgb_main_scene = 0;

  // texture paths
  const std::string sheet_kenny = "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  const std::string sheet_custom = "assets/textures/custom_spaceships.png";
  const std::string sheet_sprout = "assets/textures/sprout_lands/chars/basic_char.png";
  const std::string sheet_logo = "assets/textures/logo.png";
  const std::string sheet_map = "assets/textures/maps/0.jpg";

  // sprite-yml info
  const std::string yml_kenny = "assets/config/spritemap_kennynl.yml";
  const std::string yml_custom = "assets/config/spritemap_custom.yml";
  const std::string yml_sprout = "assets/config/spritemap_sprout.yml";
};

} // namespace game2d