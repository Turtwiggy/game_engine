#pragma once

#include "opengl/texture.hpp"
#include "renderer/helpers.hpp"

#include <string>
#include <vector>

namespace game2d {

// textures loaded from disk
enum class AvailableTexture
{
  kenny,
};

struct SINGLETON_Textures
{
  std::vector<Texture> textures;
};

void
init_textures(SINGLETON_Textures& textures);

Texture&
get_tex(SINGLETON_Textures& e, const AvailableTexture& type);

int
get_tex_id(const SINGLETON_Textures& e, const AvailableTexture& type);

int
get_tex_unit(const SINGLETON_Textures& e, const AvailableTexture& type);

} // namespace game2d