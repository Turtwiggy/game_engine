#include "textures.hpp"

#include <algorithm>
#include <utility>

namespace game2d {

void
game2d::init_textures(SINGLETON_Textures& textures)
{
  auto& default_textures = textures.textures;

  // load textures
  std::vector<std::pair<int, std::string>> textures_to_load;

  // iterate over user config
  std::vector<Texture> results;
  for (auto& tex : default_textures) {
    tex.tex_unit = request_texture(results, tex.path, tex.spritesheet_path).unit;
    textures_to_load.emplace_back(tex.tex_unit.unit, tex.path);
  }

#if defined(__EMSCRIPTEN__)
  auto tex_ids = engine::load_textures(textures_to_load);
#else
  auto tex_ids = engine::load_textures_threaded(textures_to_load);
#endif

  for (int i = 0; auto& tex : default_textures) {

    // find bound id
    tex.tex_id.id = tex_ids[i];

    std::cout << "set " << tex.path << " to tex_id: " << tex.tex_id.id << " unit: " << tex.tex_unit.unit << "\n";

    i++;
  }
}

Texture&
get_tex(SINGLETON_Textures& e, const AvailableTexture& type)
{
  auto& textures = e.textures;

  auto type_as_id = static_cast<int>(type);
  auto it = std::find_if(
    textures.begin(), textures.end(), [&type_as_id, &type](const Texture& a) { return a.tex_unit.unit == type_as_id; });

  return *it;
};

int
get_tex_id(const SINGLETON_Textures& e, const AvailableTexture& type)
{
  const auto& textures = e.textures;

  auto type_as_id = static_cast<int>(type);
  auto it = std::find_if(
    textures.begin(), textures.end(), [&type_as_id, &type](const Texture& a) { return a.tex_unit.unit == type_as_id; });

  // warning: could be null if not found via name
  return it->tex_id.id;
};

int
get_tex_unit(const SINGLETON_Textures& e, const AvailableTexture& type)
{
  const auto& textures = e.textures;

  auto type_as_id = static_cast<int>(type);
  auto it = std::find_if(
    textures.begin(), textures.end(), [&type_as_id, &type](const Texture& a) { return a.tex_unit.unit == type_as_id; });

  // warning: could be null if not found via name
  return it->tex_unit.unit;
};

} // namespace game2d