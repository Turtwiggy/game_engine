#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "maths/grid.hpp"
#include "modules/actor_bodypart_legs/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_enemy_patrol/helpers.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_procedural/cell_automata.hpp"
#include "modules/algorithm_procedural/poisson.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/debug_pathfinding/components.hpp"
#include "modules/entt/serialize.hpp"
#include "modules/gameover/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/sprite_spritestack/components.hpp"
#include "modules/ui_arrows_to_spawners/components.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_level_up/components.hpp"
#include "modules/ui_rpg_character/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"
#include <nlohmann/json.hpp>

#include <string>

namespace game2d {

using namespace std::literals;
using json = nlohmann::json;

entt::entity
create_player(entt::registry& r, const glm::ivec2& pos)
{
  // create weapon before player to draw on top
  const auto weapon = create_gameplay(r, EntityType::weapon_shotgun);

  // player
  const auto e = create_gameplay(r, EntityType::actor_player);
  set_position(r, e, pos);

  // setup weapon
  auto& weapon_parent = r.get<HasParentComponent>(weapon);
  weapon_parent.parent = e;
  set_position(r, weapon, r.get<AABB>(e).center);

  // link player&weapon
  HasWeaponComponent has_weapon;
  has_weapon.instance = weapon;
  r.emplace<HasWeaponComponent>(e, has_weapon);

  // give legs
  auto legs_e = create_gameplay(r, EntityType::actor_bodypart_leg);
  auto& legs = r.get<LegsComponent>(legs_e);
  legs.body = e;

  // give head
  auto head = create_gameplay(r, EntityType::actor_bodypart_head);
  r.emplace<HasParentComponent>(head, e);

  return e;
}

entt::entity
create_player_ally(entt::registry& r, const entt::entity& group)
{
  const auto& group_pos = r.get<AABB>(group);

  // create weapon before player to draw on top
  const auto weapon = create_gameplay(r, EntityType::weapon_shotgun);
  set_position(r, weapon, group_pos.center);

  // player ally
  const auto e = create_gameplay(r, EntityType::actor_player_ally);
  set_position(r, e, group_pos.center);

  // link group&player
  r.emplace<HasParentComponent>(e, group);

  // weapon
  auto& weapon_parent = r.get<HasParentComponent>(weapon);
  weapon_parent.parent = e;

  // link player&weapon
  HasWeaponComponent has_weapon;
  has_weapon.instance = weapon;
  r.emplace<HasWeaponComponent>(e, has_weapon);

  return e;
};

void
add_boundary_walls(entt::registry& r, const int w, const int h, const int tilesize)
{
  const int half_tile_size = tilesize / 10.0f;
  const auto wall_l = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_l, { 0, h / 2.0f });
  set_size(r, wall_l, { half_tile_size, h });
  const auto wall_r = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_r, { w, h / 2.0f });
  set_size(r, wall_r, { half_tile_size, h });
  const auto wall_u = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_u, { w / 2.0f, 0 });
  set_size(r, wall_u, { w, half_tile_size });
  const auto wall_d = create_gameplay(r, EntityType::solid_wall);
  set_position(r, wall_d, { w / 2.0f, h });
  set_size(r, wall_d, { w, half_tile_size });
}

void
move_to_scene_start(entt::registry& r, const Scene s, const bool load_saved)
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
  destroy_and_create<SINGLE_ScreenshakeComponent>(r);
  destroy_and_create<SINGLE_SelectedUI>(r);
  destroy_and_create<SINGLE_ArrowsToSpawnerUI>(r);
  destroy_and_create<SINGLE_UILevelUpComponent>(r);
  destroy_and_create<SINGLE_TurnBasedCombatInfo>(r);
  destroy_and_create<SINGLE_DebugPathLines>(r);
  destroy<MapComponent>(r);
  // SINGLE_MainMenuUI: not destroyed. destroyed if scene is menu.
  destroy<Effect_GridComponent>(r);

  // Clear out any old input
  destroy_and_create<SINGLETON_InputComponent>(r);
  init_input_system(r);

  // HACK: the first and only transform should be the camera
  auto& camera = get_first_component<TransformComponent>(r);
  camera.position = glm::ivec3(0, 0, 0);

  // create a cursor
  create_gameplay(r, EntityType::cursor);

  stop_all_audio(r);

  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

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

  const auto get_seed_from_systemtime = []() -> time_t {
    auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
  };

  if (s == Scene::game) {
    const auto& menu_ui = get_first_component<SINGLE_MainMenuUI>(r);

    // Play some audio
    r.emplace<AudioRequestPlayEvent>(r.create(), "GAME_01");

    // create cursor debugs
    const auto cursor = get_first<CursorComponent>(r);
    r.get<SpriteComponent>(cursor).colour.a = 0.1f;

    // create background effect
    r.emplace<Effect_GridComponent>(r.create());

    // create a hostile dummy
    // {
    //   const auto e = create_gameplay(r, EntityType::enemy_dummy);
    //   auto& e_aabb = r.get<AABB>(e);
    //   e_aabb.size = default_size * 1;
    //   e_aabb.center = { 200, 0 };
    // }

    // create a group with one unit in
    {
      const auto& player_group = create_gameplay(r, EntityType::actor_unitgroup);
      set_position(r, player_group, { 128, 128 });
      // group_aabb.center = { 32, 32 };
      create_player_ally(r, player_group);
      create_player_ally(r, player_group);
    }

    // create players based off main menu ui
    // note: create player before other sprites to render on top
    //
    const auto& character_stats_view = r.view<CharacterStats, InActiveFight>();
    for (const auto& [request_e, stats_e, fight_e] : character_stats_view.each()) {

      const auto player = create_player(r, { 32, 32 });

      // if in game...
      r.emplace<CameraFollow>(player);
    }

    // map width and height
    // out of which is considered out of bounds
    const int width = 1024;
    const int height = 1024;

    // set camera
    const auto& camera = get_first<OrthographicCamera>(r);
    auto& camera_t = r.get<TransformComponent>(camera);
    camera_t.position.x = width / 2.0f;
    camera_t.position.y = height / 2.0f;

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

        // HACK: just so different every playthrough
        const int seed = get_seed_from_systemtime();

        auto map = generate_50_50({ map_c.xmax, map_c.ymax }, seed);
        map = iterate_with_cell_automata(map, { map_c.xmax, map_c.ymax });
        map = iterate_with_cell_automata(map, { map_c.xmax, map_c.ymax });
        map = iterate_with_cell_automata(map, { map_c.xmax, map_c.ymax });

        // conver map 0s and 1s to entity map
        map_c.map.resize(map.size());
        auto& maap = map_c.map;
        for (int i = 0; i < maap.size(); i++) {
          auto xy_world = engine::grid::index_to_world_position(i, map_c.xmax, map_c.ymax, map_c.tilesize);
          xy_world += map_offset;

          // wall
          if (map[i] == 1) {
            const auto e = create_gameplay(r, EntityType::solid_wall);
            // set_sprite_custom(r, e, "icon_beer"s, tex_unit_for_bargame);
            r.get<AABB>(e).center = xy_world;
            r.get<TransformComponent>(e).scale = { map_c.tilesize, map_c.tilesize, 1.0f };
            r.emplace<PathfindComponent>(e, -1);

            maap[i].push_back(e);
          }
          // floor
          else
            maap[i].push_back(entt::null);
        }
        r.emplace<MapComponent>(r.create(), map_c);
      }
    }

    // Create an enemy
    {
      // const auto e = create_gameplay(r, EntityType::enemy_grunt);
      // set_position(r, e, { 64, 64 });
    }

    // Generate "seed points" for bases
    bool seed_spawners = true;
    MapComponent spawners;
    if (seed_spawners) {
      const auto& map = get_first_component<MapComponent>(r);
      spawners.map.resize((map.map.size()));

      // HACK: just so different every playthrough
      const int seed = get_seed_from_systemtime();

      const int poisson_space_for_spawners = 250;
      const int distance_between_points = poisson_space_for_spawners;
      const auto poisson = generate_poisson(width, height, distance_between_points, seed);
      std::cout << "generated " << poisson.size() << " poisson points for bases" << std::endl;
      for (const auto& p : poisson) {

        // check if gridspace is in a wall...
        const auto poisson_point_gridspace = engine::grid::world_space_to_grid_space(p, map.tilesize);
        const auto poisson_point_idx = engine::grid::grid_position_to_index(poisson_point_gridspace, map.xmax);
        const auto& map_entity = map.map[poisson_point_idx];
        const auto& first_map_entity = map_entity[0];
        if (first_map_entity != entt::null && r.get<PathfindComponent>(first_map_entity).cost == -1) {
          // do not spawn on this point... its blocked
          std::cout << "not spawning spawnpoint on blocked gridspace" << std::endl;
          continue;
        }

        // create a spawner at these positions
        const auto e = create_gameplay(r, EntityType::actor_spawner);
        r.emplace<OnlySpawnInRangeOfAnyPlayerComponent>(e);
        auto& e_aabb = r.get<AABB>(e);
        e_aabb.size = { 32, 32 };

        // make sure seed points are clamped to grid space
        auto poisson_point_clamped = engine::grid::grid_space_to_world_space(poisson_point_gridspace, map.tilesize);
        poisson_point_clamped.x += map.tilesize / 2.0f; // center, not top left
        poisson_point_clamped.y += map.tilesize / 2.0f; // center, not top left
        e_aabb.center = { poisson_point_clamped.x, poisson_point_clamped.y };

        auto& spawner = r.get<SpawnerComponent>(e);
        spawner.types_to_spawn.push_back(EntityType::enemy_grunt);
        spawner.types_to_spawn.push_back(EntityType::enemy_ranged);

        // Create a spawner map, just so e.g. "barricades" dont spawn in the same place
        spawners.map[poisson_point_idx].push_back(e);
      }
    }

    // Generate "seed points" for bases
    bool seed_barricades = true;
    if (seed_barricades) {
      const auto& map = get_first_component<MapComponent>(r);
      const int seed = 1;
      const int poisson_space_for_barricades = 200;
      const int distance_between_points = poisson_space_for_barricades;
      const auto poisson = generate_poisson(width, height, distance_between_points, seed);
      std::cout << "generated " << poisson.size() << " poisson points for bases" << std::endl;
      for (const auto& p : poisson) {

        // check if map gridspace is full with wall...
        const auto poisson_point_gridspace = engine::grid::world_space_to_grid_space(p, map.tilesize);
        const auto poisson_point_idx = engine::grid::grid_position_to_index(poisson_point_gridspace, map.xmax);
        const auto& map_entity = map.map[poisson_point_idx];
        const auto& first_map_entity = map_entity[0];
        if (first_map_entity != entt::null && r.get<PathfindComponent>(first_map_entity).cost == -1) {
          std::cout << "not spawning barricade on blocked gridspace" << std::endl;
          continue;
        }

        // check if map gridspace is full with spawner...
        const auto& map_entity_spawner = spawners.map[poisson_point_idx];
        if (map_entity_spawner.size() > 0)
          continue;

        // create at these positions
        const auto e = create_gameplay(r, EntityType::actor_barricade);
        auto& e_aabb = r.get<AABB>(e);
        e_aabb.size = { 64, 64 };

        // make sure seed points are clamped to grid space
        auto poisson_point_clamped = engine::grid::grid_space_to_world_space(poisson_point_gridspace, map.tilesize);
        poisson_point_clamped.x += map.tilesize / 2.0f; // center, not top left
        poisson_point_clamped.y += map.tilesize / 2.0f; // center, not top left
        e_aabb.center = { poisson_point_clamped.x, poisson_point_clamped.y };
      }
    }

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, width, height, 32);

    // VISUAL: use poisson for grass
    {
      // const int tex_unit_for_bargame = search_for_texture_unit_by_texture_path(ri, "bargame")->unit;
      // const auto poisson = generate_poisson(width, height, 150, 0);
      // std::cout << "generated " << poisson.size() << " poisson points" << std::endl;
      // for (const auto& p : poisson) {
      //   const auto icon = create_gameplay(r, EntityType::empty_with_transform);
      //   set_sprite_custom(r, icon, "icon_grass"s, tex_unit_for_bargame);

      //   r.get<TransformComponent>(icon).position = { p.x, p.y, 0.0f };
      //   r.get<TagComponent>(icon).tag = "grass"s;
      // }
    }
  }

  if (s == Scene::test_scene_gun) {
    int width = 1000;
    int height = 1000;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = width / map_c.tilesize;
    map_c.ymax = height / map_c.tilesize;
    const glm::ivec2 tilesize{ map_c.tilesize, map_c.tilesize };
    const glm::ivec2 map_offset = { tilesize.x / 2.0f, tilesize.y / 2.0f };
    r.emplace<MapComponent>(r.create(), map_c);

    const auto player = create_player(r, { 128, 128 });
    r.emplace<CameraFollow>(player);

    const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "monochrome").value();
    const auto create_wall_piece = [&r, &tex_unit](const glm::vec2& pos) {
      const int sprites_for_total_sprite = 10;
      entt::entity root_entity = entt::null;
      for (int i = 0; i < sprites_for_total_sprite; i++) {
        const auto i_as_str = std::to_string(i);
        entt::entity sprite_e = entt::null;
        if (i == 0) {
          sprite_e = create_gameplay(r, EntityType::solid_wall);
          root_entity = sprite_e;
          set_position(r, sprite_e, pos);
        } else
          sprite_e = create_gameplay(r, EntityType::empty_with_transform);
        set_sprite_custom(r, sprite_e, "EMPTY", tex_unit.unit);
        SpritestackComponent spritestack(i);
        spritestack.spritestack_total = sprites_for_total_sprite;
        if (i > 0)
          spritestack.root = root_entity;
        r.emplace<SpritestackComponent>(sprite_e, spritestack);
      }
    };

    // top row
    for (int i = 0; i < 10; i++)
      create_wall_piece({ 32 * i, 0 });

    // left row
    for (int i = 1; i < 10; i++)
      create_wall_piece({ 0, 32 * i });

    // right row
    for (int i = 1; i < 10; i++)
      create_wall_piece({ 32 * 10, 32 * i });

    // bottom row
    for (int i = 0; i < 10; i++)
      create_wall_piece({ 32 * i, 32 * 10 });
  }

  if (s == Scene::dungeon_designer) {
    // create background effect
    r.emplace<Effect_GridComponent>(r.create());

    // set dungeon constraints
    int map_width = 2000;
    int map_height = 2000;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    r.emplace<MapComponent>(r.create(), map_c);

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // VISUAL: use poisson for grass
    // {
    //   int width = 1000;
    //   int height = 1000;
    //   const int tex_unit_for_bargame = search_for_texture_unit_by_texture_path(ri, "bargame")->unit;
    //   const auto poisson = generate_poisson(width, height, 150, 0);
    //   std::cout << "generated " << poisson.size() << " poisson points" << std::endl;
    //   for (const auto& p : poisson) {
    //     const auto icon = create_gameplay(r, EntityType::empty_with_transform);
    //     set_sprite_custom(r, icon, "icon_grass"s, tex_unit_for_bargame);
    //     r.get<TransformComponent>(icon).position = { p.x, p.y, 0.0f };
    //     r.get<TagComponent>(icon).tag = "grass"s;
    //   }
    // }
  }

  if (s == Scene::duckgame_overworld) {

    int map_width = 1000;
    int map_height = 1000;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    r.emplace<MapComponent>(r.create(), map_c);

    // grid effect
    r.emplace<Effect_GridComponent>(r.create(), map_c.tilesize);

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // Add respawner without body
    // bool add_spawner = false;
    // if (add_spawner) {
    //   SpawnerComponent spawner_c;
    //   spawner_c.types_to_spawn = { EntityType::actor_enemy_patrol };
    //   spawner_c.continuous_spawn = true;
    //   AABB spawner_area;
    //   // area: entire map
    //   spawner_area.center = { map_width / 2.0f, map_height / 2.0f };
    //   spawner_area.size = { map_width, map_height };
    //   spawner_c.spawn_in_boundingbox = true;
    //   spawner_c.spawn_area = spawner_area;
    //   const auto e = create_gameplay(r, EntityType::actor_spawner);
    //   r.emplace_or_replace<SpawnerComponent>(e, spawner_c);
    //   // not visible
    //   r.remove<TransformComponent>(e);
    //   // stop being physics actor
    //   r.remove<PhysicsTransformXComponent>(e);
    //   r.remove<PhysicsTransformYComponent>(e);
    //   r.remove<AABB>(e);
    //   r.remove<PhysicsActorComponent>(e);
    //   r.remove<VelocityComponent>(e);
    // }

    //
    // spawnables below
    //

    if (load_saved)
      load_if_exists(r, "save-overworld.dat");
    else {
      const glm::ivec2 home_base_position = { 25, 25 };

      const auto player = create_gameplay(r, EntityType::actor_player);
      set_position(r, player, home_base_position);
      r.emplace<CameraFollow>(player);

      // TODO: BAD. FIX.
      static engine::RandomState rnd;

      for (int i = 0; i < 20; i++) {
        const auto enemy = create_gameplay(r, EntityType::actor_enemy_patrol);

        PatrolDescription desc;
        update_patrol_from_desc(r, enemy, desc);

        // random position, dont spawn at 0, 0
        const int rnd_x = int(engine::rand_det_s(rnd.rng, 1, (map_c.xmax - 1)));
        const int rnd_y = int(engine::rand_det_s(rnd.rng, 1, (map_c.ymax - 1)));
        set_position(r, enemy, { rnd_x * map_c.tilesize, rnd_y * map_c.tilesize });
      }
    }

    //
  }

  if (s == Scene::turnbasedcombat) {

    int map_width = 1000;
    int map_height = 1000;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    r.emplace<MapComponent>(r.create(), map_c);
    r.emplace<Effect_GridComponent>(r.create(), map_c.tilesize);

    // player sizes
    const glm::ivec2 size{ 20, 20 };

    // create player team
    for (int i = 0; i < 5; i++) {

      // create weapon before player to draw on top
      const auto weapon = create_gameplay(r, EntityType::weapon_shotgun);

      // player
      const auto e = create_gameplay(r, EntityType::actor_unit_rtslike);
      r.get<TeamComponent>(e).team = AvailableTeams::player;
      const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
      const auto pos = glm::ivec2{ map_c.tilesize * 2, (i + 1) * (map_c.tilesize * 2) } + offset;
      set_position(r, e, pos);
      set_size(r, e, size);

      // setup weapon
      auto& weapon_parent = r.get<HasParentComponent>(weapon);
      weapon_parent.parent = e;
      set_position(r, weapon, r.get<AABB>(e).center);

      // link player&weapon
      HasWeaponComponent has_weapon;
      has_weapon.instance = weapon;
      r.emplace<HasWeaponComponent>(e, has_weapon);

      r.emplace<DefaultColour>(e, engine::SRGBColour{ 0.0f, 0.3f, 0.8f, 1.0f });
      r.emplace<HoveredColour>(e, engine::SRGBColour{ 0.0f, 1.0f, 1.0f, 1.0f });
      set_colour(r, e, r.get<DefaultColour>(e).colour);
    }

    // create enemy team
    for (int i = 0; i < 5; i++) {
      const auto e = create_gameplay(r, EntityType::actor_unit_rtslike);
      r.get<TeamComponent>(e).team = AvailableTeams::enemy;

      // GAMEDESIGN TODO:
      // set out of range, so that player is free-roaming, then
      // when they get in range, the "turn-based combat starts"

      const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
      const auto pos = glm::ivec2{ map_c.tilesize * 10, (i + 1) * (map_c.tilesize * 2) } + offset;
      set_position(r, e, pos);
      set_size(r, e, size);

      r.emplace<DefaultColour>(e, engine::SRGBColour{ 0.8f, 0.3f, 0.0f, 1.0f });
      r.emplace<HoveredColour>(e, engine::SRGBColour{ 1.0f, 1.0f, 0.0f, 1.0f });
      set_colour(r, e, r.get<DefaultColour>(e).colour);
    }

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.to_place_debug = create_gameplay(r, EntityType::empty_with_transform);
    info.action_cursor = create_gameplay(r, EntityType::empty_with_transform);
    set_size(r, info.to_place_debug, { 0, 0 }); // start disabled
    set_size(r, info.action_cursor, { 0, 0 });  // start disabled
  }

  const auto scene_name = std::string(magic_enum::enum_name(s));
  std::cout << "setting scene to: " << scene_name << std::endl;
  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d