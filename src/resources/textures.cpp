#include "textures.hpp"

#include <entt/entt.hpp>

#include <algorithm>

namespace game2d {

Texture&
get_tex(entt::registry& r, const TextureType& type)
{
  auto& textures = r.ctx().at<SINGLETON_Textures>().textures;
  auto it = std::find_if(textures.begin(), textures.end(), [&type](const auto& a) { return a.type == type; });
  return *it;
};

int
get_tex_id(entt::registry& r, const TextureType& type)
{
  auto& textures = r.ctx().at<SINGLETON_Textures>().textures;
  auto it = std::find_if(textures.begin(), textures.end(), [&type](const auto& a) { return a.type == type; });

  // warning: could be null if not found via name
  return it->tex_id;
};

int
get_tex_unit(entt::registry& r, const TextureType& type)
{
  auto& textures = r.ctx().at<SINGLETON_Textures>().textures;
  auto it = std::find_if(textures.begin(), textures.end(), [&type](const auto& a) { return a.type == type; });

  // warning: could be null if not found via name
  return it->tex_unit;
};

} // namespace game2d