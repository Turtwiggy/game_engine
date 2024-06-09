#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/helpers.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "entt/serialize.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "maths/grid.hpp"
#include "modules/actor_bodypart_legs/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_enemy_patrol/helpers.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/algorithm_procedural/cell_automata.hpp"
#include "modules/algorithm_procedural/poisson.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/debug_pathfinding/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/sprite_spritestack/components.hpp"
#include "modules/system_minigame_bamboo/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_arrows_to_spawners/components.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_level_up/components.hpp"
#include "modules/ui_rpg_character/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
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
create_combat_entity(entt::registry& r, const CombatEntityDescription& desc)
{
  const auto& pos = desc.position;

  // create weapon before player to draw on top
  const auto weapon = create_gameplay(r, EntityType::weapon_shotgun);
  r.emplace_or_replace<TeamComponent>(weapon, desc.team);

  // base entity
  const auto e = create_gameplay(r, EntityType::actor_unit_rtslike);
  set_position(r, e, pos);
  r.emplace_or_replace<TeamComponent>(e, desc.team);

  // set entity to aim by default to the right
  r.emplace<StaticTargetComponent>(e, glm::ivec2{ desc.position.x + 100, desc.position.y });

  // setup weapon
  auto& weapon_parent = r.get<HasParentComponent>(weapon);
  weapon_parent.parent = e;
  set_position(r, weapon, r.get<AABB>(e).center);
  move_entity_on_map(r, e, pos);

  // link player&weapon
  HasWeaponComponent has_weapon;
  has_weapon.instance = weapon;
  r.emplace<HasWeaponComponent>(e, has_weapon);

  if (desc.team == AvailableTeams::player) {
    r.emplace<DefaultColour>(e, engine::SRGBColour{ 0.0f, 0.3f, 0.8f, 1.0f });
    r.emplace<HoveredColour>(e, engine::SRGBColour{ 0.0f, 1.0f, 1.0f, 1.0f });
    set_colour(r, e, r.get<DefaultColour>(e).colour);
  } else if (desc.team == AvailableTeams::enemy) {
    r.emplace<DefaultColour>(e, engine::SRGBColour{ 0.8f, 0.3f, 0.0f, 1.0f });
    r.emplace<HoveredColour>(e, engine::SRGBColour{ 1.0f, 0.0f, 0.0f, 1.0f });
    set_colour(r, e, r.get<DefaultColour>(e).colour);
    r.emplace_or_replace<EnemyComponent>(e);
  }

  set_sprite(r, e, "PERSON_25_2");

  return e;
}

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

  const auto& ui = r.view<WorldspaceTextComponent>();
  r.destroy(ui.begin(), ui.end());

  destroy_first_and_create<SINGLETON_CurrentScene>(r);
  destroy_first_and_create<SINGLETON_PhysicsComponent>(r);
  destroy_first_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_first_and_create<SINGLETON_GameStateComponent>(r);
  destroy_first_and_create<SINGLETON_GameOver>(r);
  destroy_first_and_create<SINGLE_ScreenshakeComponent>(r);
  destroy_first_and_create<SINGLE_SelectedUI>(r);
  destroy_first_and_create<SINGLE_ArrowsToSpawnerUI>(r);
  destroy_first_and_create<SINGLE_UILevelUpComponent>(r);
  destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);
  destroy_first_and_create<SINGLE_DebugPathLines>(r);
  destroy_first_and_create<SINGLE_CombatState>(r);
  destroy_first<MapComponent>(r);
  // SINGLE_MainMenuUI: not destroyed. destroyed if scene is menu.
  destroy_first<Effect_GridComponent>(r);
  destroy_first<SINGLE_MinigameBamboo>(r);
  // Clear out any old input
  destroy_first_and_create<SINGLETON_InputComponent>(r);
  init_input_system(r);

  // HACK: the first and only transform should be the camera
  const auto camera_e = get_first<TransformComponent>(r);
  if (auto* can_move = r.try_get<CameraFreeMove>(camera_e))
    r.remove<CameraFreeMove>(camera_e); // reset to default
  auto& camera = r.get<TransformComponent>(camera_e);
  camera.position = glm::ivec3(0, 0, 0);

  // create a cursor
  const auto cursor_e = create_gameplay(r, EntityType::cursor);
  set_size(r, cursor_e, { 0, 0 });

  stop_all_audio(r);

  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  if (s == Scene::menu) {
    auto& ui = destroy_first_and_create<SINGLE_MainMenuUI>(r);

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

    // create a background sprite
    {
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "background_mainmenu").value();
      set_sprite_custom(r, e, "SPACE_BACKGROUND_0", tex_unit.unit);
      set_size(r, e, { 3000, 3000 });
      set_position(r, e, { 0, -600 });
      r.get<TransformComponent>(e).position.z = -2; // behind everything
    }
  }

  // const auto get_seed_from_systemtime = []() -> time_t {
  //   auto now = std::chrono::system_clock::now();
  //   return std::chrono::system_clock::to_time_t(now);
  // };

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

    CombatEntityDescription desc;
    desc.position = { 128, 128 };
    desc.team = AvailableTeams::player;
    const auto e = create_combat_entity(r, desc);

    const auto player = e;
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

  if (s == Scene::duckgame_overworld) {
    // Play some audio
    // TODO: check if muted...
    r.emplace<AudioRequestPlayEvent>(r.create(), "GAME_01");

    int map_width = 2000;
    int map_height = 2000;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    r.emplace<MapComponent>(r.create(), map_c);

    // create a piece of worldspace text for the quadrant
    {
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      auto& ui = r.emplace<WorldspaceTextComponent>(e);
      ui.text = "Quadrant 4.2UNE";
      set_position(r, e, { 45, -10 });
      set_size(r, e, { 0, 0 });
    };

    // create a background sprite
    {
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "background_0").value();
      set_sprite_custom(r, e, "SPACE_BACKGROUND_0", tex_unit.unit);
      set_size(r, e, { 3000, 3000 });
      set_position(r, e, { map_width / 2, map_height / 2 }); // center
      r.get<TransformComponent>(e).position.z = -2;          // behind everything
    }

    // create an empty sprite on the board sprite
    // {
    //   const auto e = create_gameplay(r, EntityType::empty_with_transform);
    //   set_sprite(r, e, "EMPTY");
    //   set_size(r, e, { map_width, map_height });
    //   set_position(r, e, { map_width / 2, map_height / 2 }); // center
    //   set_colour(r, e, engine::SRGBColour{ 0.3f, 0.3f, 0.3f, 0.1f });
    //   r.get<TransformComponent>(e).position.z = -1; // behind everything
    // }

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // Add respawner without body
    bool add_spawner = false;
    if (add_spawner) {
      SpawnerComponent spawner_c;
      spawner_c.types_to_spawn = { EntityType::actor_enemy_patrol };
      spawner_c.continuous_spawn = true;
      AABB spawner_area;
      // area: entire map
      spawner_area.center = { map_width / 2.0f, map_height / 2.0f };
      spawner_area.size = { map_width, map_height };
      spawner_c.spawn_in_boundingbox = true;
      spawner_c.spawn_area = spawner_area;
      const auto e = create_gameplay(r, EntityType::actor_spawner);
      r.emplace_or_replace<SpawnerComponent>(e, spawner_c);
      // not visible
      r.remove<TransformComponent>(e);
      // stop being physics actor
      r.remove<PhysicsTransformXComponent>(e);
      r.remove<PhysicsTransformYComponent>(e);
      r.remove<AABB>(e);
      r.remove<PhysicsActorComponent>(e);
      r.remove<VelocityComponent>(e);
    }

    // spawnables below
    //
    if (load_saved)
      load_if_exists(r, "save-overworld.json");
    else {
      // const int seed = get_seed_from_systemtime();
      const int seed = 2;
      static engine::RandomState rnd(seed); // TODO: BAD. FIX.

      // spawn the player somewhere random on the map
      const int rnd_x = int(engine::rand_det_s(rnd.rng, map_c.tilesize, map_width - map_c.tilesize));
      const int rnd_y = int(engine::rand_det_s(rnd.rng, map_c.tilesize, map_height - map_c.tilesize));
      const auto pos = glm::ivec2{ rnd_x, rnd_y };

      const auto player = create_gameplay(r, EntityType::actor_player);
      set_position(r, player, pos);
      r.emplace<CameraFollow>(player);

      for (int i = 0; i < 20; i++) {
        const auto enemy = create_gameplay(r, EntityType::actor_enemy_patrol);

        PatrolDescription desc;
        update_patrol_from_desc(r, enemy, desc);

        // random position, dont spawn at 0, 0
        const int rnd_x = int(engine::rand_det_s(rnd.rng, 1, (map_c.xmax - 1)));
        const int rnd_y = int(engine::rand_det_s(rnd.rng, 1, (map_c.ymax - 1)));
        set_position(r, enemy, { rnd_x * map_c.tilesize, rnd_y * map_c.tilesize });
      }

      // HACK: create a spacestation
      const auto spacestation_e = create_gameplay(r, EntityType::empty_with_physics);
      set_size(r, spacestation_e, { 200, 200 });
      const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "spacestation_0").value();
      set_sprite_custom(r, spacestation_e, "SPACESTATION_0", tex_unit.unit);
      set_position(r, spacestation_e, { map_width / 2, map_height / 2 }); // center
      set_colour(r, spacestation_e, { 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // VISUAL: use poisson for stars?
    // {
    //   const int width = map_width;
    //   const int height = map_height;
    //   const auto poisson = generate_poisson(width, height, 150, 0);
    //   for (const auto& p : poisson) {
    //     const auto icon = create_gameplay(r, EntityType::empty_with_transform);
    //     set_sprite(r, icon, "EMPTY"s);
    //     set_size(r, icon, { 8, 8 });
    //     set_position(r, icon, { p.x, p.y });
    //     set_colour(r, icon, { 255, 225, 123, 0.35f });
    //     r.get<TagComponent>(icon).tag = "star"s;
    //   }
    // }
  }

  if (s == Scene::dungeon_designer) {
    r.emplace<SINGLE_CombatState>(r.create());
    r.emplace<CameraFreeMove>(camera_e);
    r.emplace<Effect_GridComponent>(r.create());

    // create new map & dungeon constraints
    const int map_width = 1000;
    const int map_height = 1000;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    r.emplace<MapComponent>(r.create(), map_c);
    auto& map = get_first_component<MapComponent>(r);

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_gameplay(r, EntityType::empty_with_transform);
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled

    // request to generate a dungeon
    // r.emplace<RequestGenerateDungeonComponent>(r.create());

    // HACK: pretend we hit a spaceship
    SINGLE_DuckgameToDungeon data;
    data.backstabbed = true;
    data.patrol_that_you_hit.strength = 10;
    destroy_first_and_create<SINGLE_DuckgameToDungeon>(r, data);
  }

  if (s == Scene::turnbasedcombat) {
    r.emplace<SINGLE_CombatState>(r.create());
    r.emplace<CameraFreeMove>(camera_e);

    int map_width = 600;
    int map_height = 600;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    r.emplace<MapComponent>(r.create(), map_c);
    r.emplace<Effect_GridComponent>(r.create(), map_c.tilesize);

    int players = 5;
    int enemies = 5;

    const auto& data_e = get_first<SINGLE_DuckgameToDungeon>(r);
    if (data_e != entt::null) { // you arrived from a scene
      const auto& data = get_first_component<SINGLE_DuckgameToDungeon>(r);
      enemies = data.patrol_that_you_hit.strength;
    }

    // create player team
    for (int i = 0; i < players; i++) {
      const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
      const auto pos = glm::ivec2{ map_c.tilesize * 2, (i + 1) * (map_c.tilesize * 2) } + offset;

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::player;
      const auto e = create_combat_entity(r, desc);
    }

    // place "root" enemy at "top right" of grid
    const auto& map = get_first_component<MapComponent>(r);
    const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
    const glm::ivec2 root_gridpos = { map.xmax - 1, 0 };
    const auto pos = glm::ivec2{ engine::grid::grid_space_to_world_space(root_gridpos, map.tilesize) } + offset;
    CombatEntityDescription desc;
    desc.position = pos;
    desc.team = AvailableTeams::enemy;
    enemies--;
    auto last_spawned_e = create_combat_entity(r, desc);

    // create enemy team
    for (int i = 0; i < enemies; i++) {
      // GAMEDESIGN TODO:
      // set out of range, so that player is free-roaming, then
      // when they get in range, the "turn-based combat starts"

      auto& map = get_first_component<MapComponent>(r);
      const auto& grid = map_to_grid(r);

      const int idx_next = get_lowest_cost_neighbour(r, map, grid, last_spawned_e);
      auto pos = engine::grid::index_to_world_position(idx_next, map.xmax, map.ymax, map.tilesize);
      pos += glm::ivec2{ map_c.tilesize / 2, map_c.tilesize / 2 }; // center

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::enemy;
      last_spawned_e = create_combat_entity(r, desc);
    }

    // Create 4 edges to the map
    bool create_edges = true;
    if (create_edges)
      add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_gameplay(r, EntityType::empty_with_transform);
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled
  }

  if (s == Scene::minigame_bamboo) {
    r.emplace<SINGLE_MinigameBamboo>(r.create());

    //
  }

  const auto scene_name = std::string(magic_enum::enum_name(s));
  std::cout << "setting scene to: " << scene_name << std::endl;
  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d