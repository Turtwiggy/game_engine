#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/helpers.hpp"
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
#include "modules/procedural/cell_automata.hpp"
#include "modules/procedural/poisson.hpp"
#include "modules/procedural/voronoi.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "modules/ui_inverse_kinematics/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/ux_hoverable_change_colour/components.hpp"
#include "physics/components.hpp"
#include "resources/colours.hpp"
#include "sprites/helpers.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <string>

namespace game2d {

using namespace std::literals;
using json = nlohmann::json;

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
  destroy_and_create<SINGLE_IKLines>(r);

  // HACK: the first and only transform should be the camera
  auto& camera = get_first_component<TransformComponent>(r);
  camera.position = glm::ivec3(0, 0, 0);

  // create a cursor
  const auto cursor = create_gameplay(r, EntityType::cursor);
  // r.remove<TransformComponent>(cursor);

  stop_all_audio(r);

  if (s == Scene::menu) {

    // Display some UI
    auto& ui = destroy_and_create<SINGLE_MainMenuUI>(r);

    // Play some audio
    r.emplace<AudioRequestPlayEvent>(r.create(), "MENU_01");

    // Load randoms name file
    const auto path = "assets/config/random_names.json";
    std::ifstream f(path);
    json data = json::parse(f);
    const auto names = data["names"]; // list of names

    // choose X random names, display them on the menu
    static engine::RandomState rnd;
    for (int i = 0; i < 4; i++) {
      const float rnd_f = engine::rand_det_s(rnd.rng, 0, names.size());
      const int rnd = static_cast<int>(rnd_f);
      const auto name = names[rnd];
      ui.random_names.push_back(name);
    }

    // draw 4 inactive-soliders wiggling
    const int pixel_scale_up_size = 2;
    const auto default_size = glm::ivec2{ 16 * pixel_scale_up_size, 16 * pixel_scale_up_size };
    for (int i = 0; i < 4; i++) {
      const auto e = create_gameplay(r, EntityType::actor_player);
      r.remove<PlayerComponent>(e);
      r.remove<PhysicsActorComponent>(e);
      r.emplace<ChangeColourOnHoverComponent>(e);

      // e.g. i=0,-200, i=1,0, i=2,200, i=3,400
      const auto scale = [](int i, int min, int increment) -> int { return min + increment * i; };
      const float val = scale(i, -200, 120);

      auto& aabb = r.get<AABB>(e);
      aabb.center = { val, 140 };

      auto& t = r.get<TransformComponent>(e);
      t.scale = { default_size.x, default_size.y, 0.0f };
      t.position = { aabb.center.x, aabb.center.y, 0.0f };

      WiggleUpAndDown wiggle;
      wiggle.base_position = aabb.center;
      wiggle.amplitude = 2.0f;
      r.emplace<WiggleUpAndDown>(e, wiggle);
    }
  }

  if (s == Scene::game) {
    const auto& anims = get_first_component<SINGLE_Animations>(r);
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
    const auto tex_unit = search_for_texture_unit_by_path(ri, "bargame")->unit;
    const auto& menu_ui = get_first_component<SINGLE_MainMenuUI>(r);

    const int pixel_scale_up_size = 2;
    const auto default_size = glm::ivec2{ 16 * pixel_scale_up_size, 16 * pixel_scale_up_size };

    // Play some audio
    r.emplace<AudioRequestPlayEvent>(r.create(), "GAME_01");

    // create cursor debugs
    {
      const auto cursor = get_first<CursorComponent>(r);
      auto& cursorc = get_first_component<CursorComponent>(r);
      r.get<SpriteComponent>(cursor).colour.a = 0.1f;

      cursorc.click_ent = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.click_ent).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
      r.remove<TransformComponent>(cursorc.click_ent);

      cursorc.held_ent = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.held_ent).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
      r.remove<TransformComponent>(cursorc.held_ent);

      cursorc.line_ent = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.line_ent).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
      r.remove<TransformComponent>(cursorc.line_ent);

      cursorc.dda_start = create_gameplay(r, EntityType::empty);
      cursorc.dda_intersection = create_gameplay(r, EntityType::empty);
      cursorc.dda_end = create_gameplay(r, EntityType::empty);
      r.get<SpriteComponent>(cursorc.dda_start).colour = engine::SRGBToLinear(engine::SRGBColour(1.0f, 0.0f, 0.0f, 1.0f));
      r.get<SpriteComponent>(cursorc.dda_intersection).colour =
        engine::SRGBToLinear(engine::SRGBColour(0.0f, 1.0f, 0.0f, 1.0f));
      r.get<SpriteComponent>(cursorc.dda_end).colour = engine::SRGBToLinear(engine::SRGBColour(0.0f, 0.0f, 1.0f, 1.0f));
      r.get<TransformComponent>(cursorc.dda_start).scale = { 8, 8, 1 };
      r.get<TransformComponent>(cursorc.dda_intersection).scale = { 8, 8, 1 };
      r.get<TransformComponent>(cursorc.dda_end).scale = { 8, 8, 1 };
    }

    // create a player
    for (int i = 0; i < 4; i++) {

      // player
      const auto e = create_gameplay(r, EntityType::actor_player);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.size = default_size;
      e_aabb.center = { -50, 0 + (32 * i) };
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

    // TEMP: create a spawner given the level difficulty
    for (int i = 0; i < menu_ui.level; i++) {
      const auto e = create_gameplay(r, EntityType::actor_spawner);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.size = default_size * 1;

      if (i == 0) // b
        e_aabb.center = { -100, 500 };
      if (i == 1) // r
        e_aabb.center = { 500, 0 };
      if (i == 2) // l
        e_aabb.center = { -500, 0 };
      if (i == 3) // t
        e_aabb.center = { -100, -500 };
    }

    // VISUAL: use poisson for grass
    {
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

    // generate some walls
    {
      MapComponent map_c;
      const glm::ivec2 tilesize{ map_c.tilesize, map_c.tilesize };
      map_c.map = generate_50_50(tilesize, 0);
      map_c.map = iterate_with_cell_automata(map_c.map, tilesize);
      map_c.map = iterate_with_cell_automata(map_c.map, tilesize);
      map_c.map = iterate_with_cell_automata(map_c.map, tilesize);

      // unblock spawn point
      if (map_c.map[0] == 1)
        map_c.map[0] = 0;

      r.emplace<MapComponent>(r.create(), map_c);

      const auto& xmax = map_c.xmax;
      const auto& ymax = map_c.ymax;
      const auto& maap = map_c.map;
      const glm::ivec2 offset = { tilesize.x / 2.0f, tilesize.y / 2.0f };

      for (int i = 0; i < maap.size(); i++) {
        const auto xy = engine::grid::index_to_grid_position(i, xmax, ymax);
        auto xy_world = engine::grid::index_to_world_position(i, xmax, ymax, map_c.tilesize);
        xy_world += offset;

        // wall
        if (maap[i] == 1) {
          const auto e = create_gameplay(r, EntityType::solid_wall);
          const auto icon_xy = set_sprite_custom(r, e, "icon_beer"s, tex_unit);
          r.get<AABB>(e).center = xy_world;
          r.get<TransformComponent>(e).scale = { default_size.x, default_size.y, 1.0f };
          r.get<TagComponent>(e).tag = "wall"s;
        }
        // floor
        else {
          const auto e = create_gameplay(r, EntityType::empty);
          const auto icon_xy = set_sprite_custom(r, e, "icon_grass"s, tex_unit);
          r.get<TransformComponent>(e).position = { xy_world.x, xy_world.y, 0.0f };
          r.get<TransformComponent>(e).scale = { default_size.x, default_size.y, 1.0f };
          r.get<TagComponent>(e).tag = "grass"s;
        }
      }
    }

    auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
    scene.s = s; // done
  }
}

} // namespace game2d