#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/helpers/sdl_mixer.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_enemy_patrol/components.hpp"
#include "modules/actor_enemy_patrol/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/debug_pathfinding/components.hpp"
#include "modules/entt/serialize.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_minigame_bamboo/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_colours/components.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_event_console/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"
#include <nlohmann/json.hpp>

#include <fmt/core.h>
#include <string>

namespace game2d {
using namespace audio::sdl_mixer;
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
    r.emplace_or_replace<PlayerComponent>(e);
  } else if (desc.team == AvailableTeams::enemy) {
    r.emplace<DefaultColour>(e, engine::SRGBColour{ 0.8f, 0.3f, 0.0f, 1.0f });
    r.emplace<HoveredColour>(e, engine::SRGBColour{ 1.0f, 0.0f, 0.0f, 1.0f });
    set_colour(r, e, r.get<DefaultColour>(e).colour);
    r.emplace_or_replace<EnemyComponent>(e);

    r.get<HealthComponent>(e).hp = 100;
    r.get<HealthComponent>(e).max_hp = 100;
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
  destroy_first_and_create<SINGLETON_InputComponent>(r);
  destroy_first_and_create<SINGLE_ScreenshakeComponent>(r);
  init_input_system(r); // clear old input

  destroy_first<SINGLE_SelectedUI>(r);
  destroy_first<SINGLE_TurnBasedCombatInfo>(r);
  destroy_first<SINGLE_DebugPathLines>(r);
  destroy_first<SINGLE_CombatState>(r);
  destroy_first<MapComponent>(r);
  destroy_first<Effect_GridComponent>(r);
  destroy_first<Effect_DoBloom>(r);
  destroy_first<SINGLE_MinigameBamboo>(r);
  destroy_first<SINGLE_TurnBasedCombatInfo>(r);
  destroy_first<DungeonGenerationResults>(r);
  destroy_first<SINGLE_EventConsoleLogComponent>(r);
  // destroy_first<SINGLE_MainMenuUI>(r);

  if (s != Scene::dungeon_designer)
    destroy_first<OverworldToDungeonInfo>(r);

  // systems that havent been destroyed...
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
  const auto& colours = get_first_component<SINGLE_ColoursInfo>(r);

  // HACK: the first and only transform should be the camera
  const auto camera_e = get_first<TransformComponent>(r);
  if (auto* can_move = r.try_get<CameraFreeMove>(camera_e))
    r.remove<CameraFreeMove>(camera_e); // reset to default
  auto& camera = r.get<TransformComponent>(camera_e);
  camera.position = glm::ivec3(0, 0, 0);

  stop_all_audio(r);

  if (s == Scene::splashscreen) {
    destroy_first_and_create<SINGLE_SplashScreen>(r);

    // create sprite
    {
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "blueberry").value();
      set_sprite_custom(r, e, "STUDIO_LOGO", tex_unit.unit);
      set_size(r, e, { 512, 512 });
      set_position(r, e, { 0, 0 }); // center
    }

    // create background
    {
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      set_size(r, e, { ri.viewport_size_render_at.x + 10, ri.viewport_size_render_at.y + 10 });
      set_position(r, e, { 0, 0 });              // center
      set_colour(r, e, { 231, 242, 248, 1.0f }); // logo background colour
    }
  }

  if (s == Scene::menu) {
    destroy_first_and_create<SINGLE_MainMenuUI>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01" });

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
    // {
    //   const auto e = create_gameplay(r, EntityType::empty_with_transform);
    //   const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "background_mainmenu").value();
    //   set_sprite_custom(r, e, "SPACE_BACKGROUND_0", tex_unit.unit);
    //   set_size(r, e, { 3000, 3000 });
    //   set_position(r, e, { 0, -600 });
    //   r.get<TransformComponent>(e).position.z = -2; // behind everything
    // }
  }

  const auto get_seed_from_systemtime = []() -> time_t {
    auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
  };

  if (s == Scene::overworld) {
    destroy_first<OverworldToDungeonInfo>(r); // clear here if exists
    // destroy_first_and_create<Effect_DoBloom>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "GAME_01" });

    const int map_width = 1600;
    const int map_height = 1600;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    create_empty<MapComponent>(r, map_c);

    // create a piece of worldspace text for the quadrant
    {
      const auto e = create_gameplay(r, EntityType::empty_with_transform);
      auto& ui = r.emplace<WorldspaceTextComponent>(e);
      ui.text = "Quadrant 4.2UNE";
      set_position(r, e, { 40, -10 });
      set_size(r, e, { 0, 0 });
    };

    // create a background sprite
    // {
    //   const auto e = create_gameplay(r, EntityType::empty_with_transform);
    //   const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "space_background").value();
    //   set_sprite_custom(r, e, "SPACE_BACKGROUND_0", tex_unit.unit);
    //   set_size(r, e, { 3000, 3000 });
    //   set_position(r, e, { map_width / 2, map_height / 2 }); // center
    //   r.get<TransformComponent>(e).position.z = -2;          // behind everything
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
#if defined(_DEBUG)
      const int seed = 2;
#else
      const int seed = get_seed_from_systemtime();
#endif
      static engine::RandomState rnd(seed);

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
    r.emplace_or_replace<CameraFreeMove>(get_first<OrthographicCamera>(r));
    destroy_first_and_create<SINGLE_CombatState>(r);
    destroy_first_and_create<SINGLE_EventConsoleLogComponent>(r);
    destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);
    // destroy_first_and_create<Effect_DoBloom>(r);
    destroy_first_and_create<Effect_GridComponent>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "COMBAT_01" });

    if (get_first<OverworldToDungeonInfo>(r) == entt::null) {
      fmt::println("OverworldToDungeonInfo is null; assuming launch from standalone");
      OverworldToDungeonInfo info;
      info.backstabbed = true;
      info.patrol_that_you_hit.strength = 10;
      destroy_first_and_create<OverworldToDungeonInfo>(r, info);
    }

    // TEMP: add info in the event console on how to play.
    auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);
    evts.events.push_back("Press 1 to select move action.");
    evts.events.push_back("Press 2 to select shoot action.");
    evts.events.push_back("Right click to perform action.");

    // create a cursor
    const auto cursor_e = create_gameplay(r, EntityType::cursor);
    set_size(r, cursor_e, { 0, 0 });

    // Create 4 edges to the map
    // bool create_edges = false;
    // if (create_edges)
    //   add_boundary_walls(r, map_width, map_height, map_c.tilesize);

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_gameplay(r, EntityType::empty_with_transform);
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled
  }

  if (s == Scene::turnbasedcombat) {
    r.emplace_or_replace<CameraFreeMove>(get_first<OrthographicCamera>(r));
    destroy_first_and_create<SINGLE_CombatState>(r);
    destroy_first_and_create<SINGLE_EventConsoleLogComponent>(r);
    destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);
    // destroy_first_and_create<Effect_DoBloom>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "COMBAT_01" });

    int map_width = 600;
    int map_height = 600;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);
    create_empty<MapComponent>(r, map_c);
    create_empty<Effect_GridComponent>(r, Effect_GridComponent{ map_c.tilesize });

    // create a cursor
    const auto cursor_e = create_gameplay(r, EntityType::cursor);
    set_size(r, cursor_e, { 0, 0 });

    int players = 1;
    int enemies = 1;

    // check if you started combat via a collison in e overworld
    if (get_first<OverworldToDungeonInfo>(r) != entt::null) {
      // const auto& data = get_first_component<OverworldToDungeonInfo>(r);
      // enemies = data.patrol_that_you_hit.strength;
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
    entt::entity last_spawned_e = entt::null;
    {
      const auto& map = get_first_component<MapComponent>(r);
      const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
      const glm::ivec2 root_gridpos = { map.xmax - 1, 0 };
      const auto pos = glm::ivec2{ engine::grid::grid_space_to_world_space(root_gridpos, map.tilesize) } + offset;
      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::enemy;
      enemies--;
      last_spawned_e = create_combat_entity(r, desc);
    }

    // create enemy team
    for (int i = 0; i < enemies; i++) {
      const auto& map = get_first_component<MapComponent>(r);
      const auto& grid = map_to_grid(r);

      const auto src_pos = get_position(r, last_spawned_e);
      const auto grid_pos = engine::grid::world_space_to_grid_space(src_pos, map.tilesize);
      const int src_idx = engine::grid::grid_position_to_index(grid_pos, map.xmax);

      const int idx_next = get_lowest_cost_neighbour(r, map, grid, src_idx, last_spawned_e);
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

    // Create a destructable barrel
    auto& map = get_first_component<MapComponent>(r);
    const entt::entity barrel_e = create_gameplay(r, EntityType::actor_barrel);
    const auto gridpos = glm::ivec2{ 4, 4 };
    set_position_grid(r, barrel_e, gridpos);
    std::vector<entt::entity>& ents = map.map[engine::grid::grid_position_to_index(gridpos, map.xmax)];
    ents.push_back(barrel_e);
  }

  if (s == Scene::minigame_bamboo) {
    create_empty<SINGLE_MinigameBamboo>(r);
  }

  const auto scene_name = std::string(magic_enum::enum_name(s));
  fmt::println("setting scene to: {}", scene_name);

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d