#include "textures.hpp"

#include <algorithm>
#include <utility>

namespace game2d {

void
game2d::init_textures(SINGLETON_Animations& anims, SINGLETON_Textures& textures)
{
  auto& default_textures = textures.textures;

  // load textures
  std::vector<std::pair<int, std::string>> textures_to_load;

  // iterate over user config
  std::vector<Texture> results;
  for (ReferenceTexture& ref : default_textures) {
    ref.tex.tex_unit = request_texture(results, ref.tex.path, ref.tex.spritesheet_path).unit;
    textures_to_load.emplace_back(ref.tex.tex_unit.unit, ref.tex.path);
  }

#if defined(__EMSCRIPTEN__)
  auto tex_ids = engine::load_textures(textures_to_load);
#else
  auto tex_ids = engine::load_textures_threaded(textures_to_load);
#endif

  for (int i = 0; ReferenceTexture & ref : default_textures) {

    // find bound id
    ref.tex.tex_id.id = tex_ids[i];

    std::cout << "set " << ref.tex.path << " to tex_id: " << ref.tex.tex_id.id << " unit: " << ref.tex.tex_unit.unit
              << "\n";

    // load sprite info from texture if it exists
    if (ref.tex.spritesheet_path != "")
      load_sprites(anims.animations, ref.tex.spritesheet_path);

    i++;
  }
}

ReferenceTexture&
get_tex(SINGLETON_Textures& e, const AvailableTexture& type)
{
  auto& textures = e.textures;
  auto it = std::find_if(textures.begin(), textures.end(), [&type](const auto& a) { return a.tag == type; });
  return *it;
};

int
get_tex_id(const SINGLETON_Textures& e, const AvailableTexture& type)
{
  const auto& textures = e.textures;
  auto it = std::find_if(textures.begin(), textures.end(), [&type](const auto& a) { return a.tag == type; });

  // warning: could be null if not found via name
  return it->tex.tex_id.id;
};

int
get_tex_unit(const SINGLETON_Textures& e, const AvailableTexture& type)
{
  const auto& textures = e.textures;
  auto it = std::find_if(textures.begin(), textures.end(), [&type](const auto& a) { return a.tag == type; });

  // warning: could be null if not found via name
  return it->tex.tex_unit.unit;
};

std::vector<Texture>
convert_tag_textures_to_textures(const SINGLETON_Textures& texs)
{
  std::vector<Texture> textures;

  for (const ReferenceTexture& tex : texs.textures) {
    Texture t;
    t.path = tex.tex.path;
    t.spritesheet_path = tex.tex.spritesheet_path;
    t.tex_id = tex.tex.tex_id;
    t.tex_unit = tex.tex.tex_unit;
    textures.push_back(t);
  }

  return textures;
};

} // namespace game2d