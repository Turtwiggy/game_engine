#pragma once

#include "opengl/texture.hpp"
#include "renderer/helpers.hpp"
#include "sprites/helpers.hpp"

#include <string>
#include <vector>

namespace game2d {

enum class AvailableTexture
{
  kenny,
};

struct ReferenceTexture
{
  AvailableTexture tag;
  Texture tex;

  ReferenceTexture() = default;
  ReferenceTexture(const AvailableTexture& tag, const Texture& tex)
    : tag(tag)
    , tex(tex){};
};

struct SINGLETON_Textures
{
  std::vector<ReferenceTexture> textures{
    ReferenceTexture(AvailableTexture::kenny,
                     Texture(std::string("assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png"),
                             std::string("assets/config/spritemap_kennynl.json"))),
  };
};

void
init_textures(SINGLETON_Animations& anims, SINGLETON_Textures& textures);

ReferenceTexture&
get_tex(SINGLETON_Textures& e, const AvailableTexture& type);

int
get_tex_id(const SINGLETON_Textures& e, const AvailableTexture& type);

int
get_tex_unit(const SINGLETON_Textures& e, const AvailableTexture& type);

std::vector<Texture>
convert_tag_textures_to_textures(const SINGLETON_Textures& texs);

} // namespace game2d