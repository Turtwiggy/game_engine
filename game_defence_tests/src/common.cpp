#include "common.hpp"

#include "entt/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

namespace tests {

void
default_setup(entt::registry& r)
{
  // load spritesheet info
  SINGLETON_RendererInfo ri = get_default_rendererinfo();
  SINGLE_Animations anims;
  for (const auto& tex : ri.user_textures)
    load_sprites(anims, tex.spritesheet_path);
  create_empty<SINGLE_Animations>(r, anims);
  create_empty<SINGLETON_RendererInfo>(r, ri);

  // create a camera
  const auto camera_e = create_empty<OrthographicCamera>(r);
  r.emplace<TransformComponent>(camera_e);

  move_to_scene_start(r, Scene::test);

  MapComponent map;
  map.tilesize = 50;
  map.xmax = 10;
  map.ymax = 10;
  map.map.resize(map.xmax * map.ymax);
  r.emplace<MapComponent>(r.create(), map);
};
} // namespace tests

} // namespace game2d