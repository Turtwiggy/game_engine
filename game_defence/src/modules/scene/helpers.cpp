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
#include "modules/actor_player/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/algorithm_procedural/cell_automata.hpp"
#include "modules/algorithm_procedural/poisson.hpp"
#include "modules/animation/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/items/helpers.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "modules/ui_arrows_to_spawners/components.hpp"
#include "modules/ui_level_up/components.hpp"
#include "modules/ui_rpg_character/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/ux_hoverable_change_colour/components.hpp"
#include "physics/components.hpp"
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
  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());

  const auto& actors = r.view<EntityTypeComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(actors.begin(), actors.end());

  destroy_and_create<SINGLETON_CurrentScene>(r);
  destroy_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_and_create<SINGLETON_GameStateComponent>(r);
  destroy_and_create<SINGLETON_GameOver>(r);
  destroy_and_create<SINGLETON_Wave>(r);
  destroy_and_create<SINGLE_ScreenshakeComponent>(r);
  destroy_and_create<SINGLE_SelectedUI>(r);
  destroy_and_create<SINGLE_ArrowsToSpawnerUI>(r);
  destroy_and_create<SINGLE_UILevelUpComponent>(r);

  // HACK: the first and only transform should be the camera
  auto& camera = get_first_component<TransformComponent>(r);
  camera.position = glm::ivec3(0, 0, 0);

  // create a cursor
  create_gameplay(r, EntityType::cursor);

  stop_all_audio(r);

  if (s == Scene::menu) {

    // Display some UI
    auto& ui = destroy_and_create<SINGLE_MainMenuUI>(r);

    // Play some audio
    // TODO: check if muted...
    r.emplace<AudioRequestPlayEvent>(r.create(), "MENU_01");

    // Load randoms name file
    // const auto path = "assets/config/random_names.json";
    // std::ifstream f(path);
    // json data = json::parse(f);
    // const auto names = data["names"]; // list of names

    // choose X random names, display them on the menu
    // static engine::RandomState rnd;
    // for (int i = 0; i < 4; i++) {
    //   const float rnd_f = engine::rand_det_s(rnd.rng, 0, names.size());
    //   const int rnd = static_cast<int>(rnd_f);
    //   const std::string name = names[rnd];
    //   const std::string delimiter = " ";
    //   const auto first_name = name.substr(0, name.find(delimiter));

    //   ui.random_names.push_back(first_name);
    // }
  }

  if (s == Scene::game) {
    const auto& anims = get_first_component<SINGLE_Animations>(r);
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto tex_unit_for_bargame = search_for_texture_unit_by_path(ri, "bargame")->unit;
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
    }

    // create players based off main menu ui
    // note: create player before other sprites to render on top
    //
    // todo: put player at first free slot in map.
    //
    const auto& character_stats_view = r.view<CharacterStats, InActiveFight>();
    for (int i = 0; const auto& [request_e, stats_e, fight_e] : character_stats_view.each()) {
      // player
      const auto e = create_gameplay(r, EntityType::actor_player);
      auto& e_aabb = r.get<AABB>(e);
      e_aabb.size = default_size;
      e_aabb.center = { 32, 32 + (32 * i) };
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
      auto& weapon_aabb = r.get<AABB>(weapon);
      weapon_aabb.center = e_aabb.center;

      // link player&weapon
      auto& player = r.get<PlayerComponent>(e);
      player.weapon = weapon;

      i++;
    }

    // map width and height
    // out of which is considered out of bounds
    const int width = 1024;
    const int height = 1024;
    const int poisson_space_for_spawners = 250;

    // set camera
    const auto& camera = get_first<OrthographicCamera>(r);
    auto& camera_t = r.get<TransformComponent>(camera);
    camera_t.position.x = width / 2.0f;
    camera_t.position.y = height / 2.0f;

    // create a hostile dummy
    // {
    //   const auto e = create_gameplay(r, EntityType::enemy_dummy);
    //   auto& e_aabb = r.get<AABB>(e);
    //   e_aabb.size = default_size * 1;
    //   e_aabb.center = { 200, 0 };
    // }

    // VISUAL: use poisson for grass
    {
      const auto poisson = generate_poisson(width, height, 150, 0);
      std::cout << "generated " << poisson.size() << " poisson points" << std::endl;
      for (const auto& p : poisson) {
        const auto icon = create_gameplay(r, EntityType::empty);
        set_sprite_custom(r, icon, "icon_grass"s, tex_unit_for_bargame);

        r.get<TransformComponent>(icon).position = { p.x, p.y, 0.0f };
        r.get<TransformComponent>(icon).scale = { default_size.x, default_size.y, 1.0f };
        r.get<TagComponent>(icon).tag = "grass"s;
      }
    }

    // generate some walls
    {
      MapComponent map_c;
      map_c.tilesize = 64;
      map_c.xmax = width / map_c.tilesize;
      map_c.ymax = height / map_c.tilesize;
      const glm::ivec2 tilesize{ map_c.tilesize, map_c.tilesize };
      const glm::ivec2 map_offset = { tilesize.x / 2.0f, tilesize.y / 2.0f };

      {
        // generate a map for 0s and 1s
        auto map = generate_50_50({ map_c.xmax, map_c.ymax }, 0);
        map = iterate_with_cell_automata(map, { map_c.xmax, map_c.ymax });
        map = iterate_with_cell_automata(map, { map_c.xmax, map_c.ymax });
        map = iterate_with_cell_automata(map, { map_c.xmax, map_c.ymax });

        // conver map 0s and 1s to entity map
        map_c.map.resize(map.size());
        auto& maap = map_c.map;
        for (int i = 0; i < maap.size(); i++) {
          const auto xy = engine::grid::index_to_grid_position(i, map_c.xmax, map_c.ymax);
          auto xy_world = engine::grid::index_to_world_position(i, map_c.xmax, map_c.ymax, map_c.tilesize);
          xy_world += map_offset;

          // wall
          if (map[i] == 1) {
            const auto e = create_gameplay(r, EntityType::solid_wall);
            // set_sprite_custom(r, e, "icon_beer"s, tex_unit_for_bargame);
            r.get<AABB>(e).center = xy_world;
            r.get<TransformComponent>(e).scale = { map_c.tilesize, map_c.tilesize, 1.0f };
            r.get<TagComponent>(e).tag = "wall"s;
            r.emplace<PathfindComponent>(e, -1);

            maap[i].push_back(e);
          }
          // floor
          else {
            const auto e = create_gameplay(r, EntityType::empty);
            r.emplace<PathfindComponent>(e, 0);
            set_sprite_custom(r, e, "icon_grass"s, tex_unit_for_bargame);
            r.get<TransformComponent>(e).position = { xy_world.x, xy_world.y, 0.0f };
            r.get<TransformComponent>(e).scale = { default_size.x, default_size.y, 1.0f };
            r.get<TagComponent>(e).tag = "grass"s;

            maap[i].push_back(e);
          }
        }
        r.emplace<MapComponent>(r.create(), map_c);
      }
    }

    // Generate "seed points" for bases
    //
    {
      const auto& map = get_first_component<MapComponent>(r);
      const int seed = 0;
      const int distance_between_points = poisson_space_for_spawners;
      const auto poisson = generate_poisson(width, height, distance_between_points, seed);
      std::cout << "generated " << poisson.size() << " poisson points for bases" << std::endl;
      for (const auto& p : poisson) {

        // check if gridspace is in a wall...
        const auto poisson_point_gridspace = engine::grid::world_space_to_grid_space(p, map.tilesize);
        const auto poisson_point_idx = engine::grid::grid_position_to_index(poisson_point_gridspace, map.xmax);
        const auto map_entity = map.map[poisson_point_idx];
        const auto fist_map_entity = map_entity[0];
        if (r.get<PathfindComponent>(fist_map_entity).cost == -1) {
          // do not spawn on this point... its blocked
          std::cout << "not spawning spawnpoint on blocked gridspace" << std::endl;
          continue;
        }

        // create a spawner at these positions
        const auto e = create_gameplay(r, EntityType::actor_spawner);
        r.emplace<OnlySpawnInRangeOfAnyPlayerComponent>(e);
        auto& e_aabb = r.get<AABB>(e);
        e_aabb.size = default_size * 1;

        // make sure seed points are clamped to grid space
        auto poisson_point_clamped = engine::grid::grid_space_to_world_space(poisson_point_gridspace, map.tilesize);
        poisson_point_clamped.x += map.tilesize / 2.0f; // center, not top left
        poisson_point_clamped.y += map.tilesize / 2.0f; // center, not top left
        e_aabb.center = { poisson_point_clamped.x, poisson_point_clamped.y };

        auto& spawner = r.get<SpawnerComponent>(e);
        spawner.types_to_spawn.push_back(EntityType::enemy_grunt);
        spawner.types_to_spawn.push_back(EntityType::enemy_ranged);
      }
    }

    auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
    scene.s = s; // done
  }
}

} // namespace game2d