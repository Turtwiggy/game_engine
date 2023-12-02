#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_dropoff_zone/components.hpp"
#include "modules/actor_pickup_zone/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_bow/components.hpp"
#include "modules/animation/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/procedural/poisson.hpp"
#include "modules/procedural/voronoi.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"
#include "sprites/helpers.hpp"

#include <string>

namespace game2d {
using namespace std::literals;

void
move_to_scene_start(entt::registry& r, const Scene s)
{
  {
    // entt::registry new_r;
    // transfer<SINGLE_Animations>(r, new_r);
    // transfer<SINGLETON_Textures>(r, new_r);
    // transfer<SINGLETON_AudioComponent>(r, new_r);
    // transfer<SINGLETON_RendererInfo>(r, new_r);
    // transfer<SINGLETON_InputComponent>(r, new_r);
    // transfer<SINGLETON_FixedUpdateInputHistory>(r, new_r);
    // transfer<AudioSource>(r, new_r); // multiple of these
    // r = std::move(new_r);
  }

  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());

  const auto& actors = r.view<EntityTypeComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(actors.begin(), actors.end());

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_ColoursComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  destroy_and_create<SINGLETON_Wave>(r);
  destroy_and_create<SINGLE_ScreenshakeComponent>(r);
  destroy_and_create<SINGLE_SelectedUI>(r);

  // create a cursor
  const auto cursor = create_gameplay(r, EntityType::cursor);

  if (s == Scene::game) {
    const auto& anims = get_first_component<SINGLE_Animations>(r);
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
    const auto tex_unit = search_for_texture_by_path(ri, "bargame")->unit;

    const int pixel_scale_up_size = 2;
    const auto default_size = glm::ivec2{ 16 * pixel_scale_up_size, 16 * pixel_scale_up_size };

    // create cursor debugs
    {
      const auto cursor = get_first<CursorComponent>(r);
      auto& cursorc = get_first_component<CursorComponent>(r);
      r.get<SpriteComponent>(cursor).colour.a = 0.1f;

      cursorc.click_ent = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.click_ent).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
      r.get<TransformComponent>(cursorc.click_ent).scale = { 8, 8, 1 };

      cursorc.held_ent = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.held_ent).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
      r.get<TransformComponent>(cursorc.held_ent).scale = { 8, 8, 1 };

      cursorc.line_ent = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.line_ent).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
    }

    // create a player
    for (int i = 0; i < 4; i++) {

      // player
      const auto e = create_gameplay(r, EntityType::actor_player);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.size = default_size;
      e_aabb.center = { 0, 0 + (32 * i) };
      auto& target_pos = r.get<HasTargetPositionComponent>(e);
      target_pos.position = e_aabb.center;
      // const auto icon_xy = set_sprite_custom(r, e, "player_0", tex_unit);
      r.get<TransformComponent>(e).scale = { e_aabb.size.x, e_aabb.size.y, 1.0f };

      // set colour
      auto& sc = r.get<SpriteComponent>(e);
      sc.colour = engine::SRGBToLinear(engine::SRGBColour(i / 5.0f, 0.6f, i / 5.0f, 1.0f));

      // weapon
      const auto weapon = create_gameplay(r, EntityType::weapon_shotgun);
      auto& weapon_parent = r.get<HasParentComponent>(weapon);
      weapon_parent.parent = e;
      auto& player = r.get<PlayerComponent>(e);
      player.weapon = weapon;
    }

    // create a hostile dummy
    // {
    //   const auto e = create_gameplay(r, EntityType::enemy_dummy);
    //   auto& e_aabb = r.get<AABB>(e);
    //   e_aabb.size = default_size * 1;
    //   e_aabb.center = { 200, 0 };
    // }

    // create a spawner
    {
      const auto e = create_gameplay(r, EntityType::actor_spawner);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.size = default_size * 1;
      e_aabb.center = { 0, 500 };
    }

    // use poisson for grass
    const int width = 1920;
    const int height = 1080;
    const auto poisson = generate_poisson(width, height, 150, 0);
    const glm::ivec2 offset = { -width / 2, -height / 2 };
    std::cout << "generated " << poisson.size() << " poisson points" << std::endl;

    for (const auto& p : poisson) {
      const auto icon = create_gameplay(r, EntityType::empty);
      const auto icon_xy = set_sprite_custom(r, icon, "icon_grass"s, tex_unit);

      r.get<TransformComponent>(icon).position = { offset.x + p.x, offset.y + p.y, 0.0f };
      r.get<TransformComponent>(icon).scale = { default_size.x, default_size.y, 1.0f };
      r.get<TagComponent>(icon).tag = "grass"s;
    }
  }

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d