#pragma once

#include <string>
#include <vector>

namespace game2d {

enum class AvailableTexture
{
  kenny,
  custom,
  sprout,
  logo,
  map_0,
  linear_main,
  linear_lighting,
  srgb_main,
};

struct Texture
{
  int tex_unit;
  AvailableTexture type;
  std::string path;
  std::string spritesheet_path;
  int tex_id; // set after bind
};

struct SINGLETON_Textures
{
  std::vector<Texture> textures{
    // to be loaded
    {
      0,
      AvailableTexture::kenny,
      "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png",
      "assets/config/spritemap_kennynl.json",
    },
    {
      1,
      AvailableTexture::custom,
      "assets/textures/custom_spaceships.png",
      "assets/config/spritemap_custom.json",
    },
    {
      2,
      AvailableTexture::sprout,
      "assets/textures/sprout_lands/chars/basic_char.png",
      // "assets/config/spritemap_sprout.json"
    },
    { 3, AvailableTexture::logo, "assets/textures/logo.png" },
    { 4, AvailableTexture::map_0, "assets/textures/maps/0.jpg" },
    // used in fbo
    { 5, AvailableTexture::linear_main },
    { 6, AvailableTexture::linear_lighting },
    { 7, AvailableTexture::srgb_main },
  };
};

Texture&
get_tex(SINGLETON_Textures& e, const AvailableTexture& type);

int
get_tex_id(const SINGLETON_Textures& e, const AvailableTexture& type);

int
get_tex_unit(const SINGLETON_Textures& e, const AvailableTexture& type);

} // namespace game2d