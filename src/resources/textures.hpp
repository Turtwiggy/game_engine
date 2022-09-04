#pragma once

#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace game2d {

enum class AvailableTexture
{
  KENNY,
  CUSTOM,
  SPROUT,
  LOGO,
  MAP_0,
  LINEAR_MAIN,
  LINEAR_LIGHTING,
  SRGB_MAIN,
};

struct Texture
{
  int tex_unit;
  AvailableTexture type;
  std::string path;
  std::string spritesheet_path;
  int tex_id; // set after bind
};

Texture&
get_tex(entt::registry& r, const AvailableTexture& type);

int
get_tex_id(entt::registry& r, const AvailableTexture& type);

int
get_tex_unit(entt::registry& r, const AvailableTexture& type);

struct SINGLETON_Textures
{
  std::vector<Texture> textures{
    // to be loaded
    {
      0,
      AvailableTexture::KENNY,
      "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png",
      "assets/config/spritemap_kennynl.json",
    },
    {
      1,
      AvailableTexture::CUSTOM,
      "assets/textures/custom_spaceships.png",
      "assets/config/spritemap_custom.json",
    },
    {
      2,
      AvailableTexture::SPROUT,
      "assets/textures/sprout_lands/chars/basic_char.png",
      // "assets/config/spritemap_sprout.json"
    },
    { 3, AvailableTexture::LOGO, "assets/textures/logo.png" },
    { 4, AvailableTexture::MAP_0, "assets/textures/maps/0.jpg" },
    // used in fbo
    { 5, AvailableTexture::LINEAR_MAIN },
    { 6, AvailableTexture::LINEAR_LIGHTING },
    { 7, AvailableTexture::SRGB_MAIN },
  };
};

} // namespace game2d