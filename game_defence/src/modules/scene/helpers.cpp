#include "helpers.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/helpers/sdl_mixer.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
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
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_change_gun_z_index/helpers.hpp"
#include "modules/system_distance_check/components.hpp"
#include "modules/system_minigame_bamboo/components.hpp"
#include "modules/system_overworld_fake_fight/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_colours/components.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_event_console/components.hpp"
#include "modules/ui_overworld_ship_label/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"
#include <box2d/b2_math.h>
#include <nlohmann/json.hpp>

#include <box2d/b2_revolute_joint.h>
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

  // base entity
  const auto e = create_gameplay(r, EntityType::actor_dungeon, pos);
  move_entity_on_map(r, e, pos);
  r.emplace_or_replace<TeamComponent>(e, TeamComponent{ desc.team });
  set_collision_filters(r, e); // needs team_component

  // movement. required for lerp?
  r.emplace<StaticTargetComponent>(e);

  // setup weapon
  const auto weapon = create_gameplay(r, EntityType::weapon_shotgun, get_position(r, e));
  r.emplace_or_replace<TeamComponent>(weapon, TeamComponent{ desc.team });

  auto& weapon_parent = r.get<HasParentComponent>(weapon);
  weapon_parent.parent = e;

  // link player&weapon
  HasWeaponComponent has_weapon;
  has_weapon.instance = weapon;
  r.emplace<HasWeaponComponent>(e, has_weapon);

  if (desc.team == AvailableTeams::player) {
    r.emplace<HoveredColour>(e, engine::SRGBColour{ 0.0f, 1.0f, 1.0f, 1.0f });
    r.emplace_or_replace<PlayerComponent>(e);
  }

  if (desc.team == AvailableTeams::enemy) {
    r.emplace<HoveredColour>(e, engine::SRGBColour{ 1.0f, 0.0f, 0.0f, 1.0f });
    r.emplace_or_replace<EnemyComponent>(e);
    r.get<HealthComponent>(e).hp = 50;
    r.get<HealthComponent>(e).max_hp = 50;

    // all enemy weapons should be able to shoot?
    r.emplace<AbleToShoot>(weapon);
  }

  set_sprite(r, e, "PERSON_25_0");
  return e;
}

void
add_boundary_walls(entt::registry& r, const int w, const int h, const int tilesize)
{
  const int half_tile_size = tilesize / 10.0f;
  const auto wall_l = create_gameplay(r, EntityType::solid_wall, { 0, h / 2.0f });
  set_size(r, wall_l, { half_tile_size, h });
  const auto wall_r = create_gameplay(r, EntityType::solid_wall, { w, h / 2.0f });
  set_size(r, wall_r, { half_tile_size, h });
  const auto wall_u = create_gameplay(r, EntityType::solid_wall, { w / 2.0f, 0 });
  set_size(r, wall_u, { w, half_tile_size });
  const auto wall_d = create_gameplay(r, EntityType::solid_wall, { w / 2.0f, h });
  set_size(r, wall_d, { w, half_tile_size });
}

void
move_to_scene_menu(entt::registry& r)
{
  destroy_first_and_create<SINGLE_MainMenuUI>(r);
  destroy_first_and_create<Effect_GridComponent>(r);
  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01" });
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  // Load randoms name file
  // const auto path = "./assets/config/random_names.json";
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

  // As the camera is at 0, 0,
  // worldspace text around the camera would be from e.g. -width/2 to width/2
  const auto half_wh = ri.viewport_size_render_at / glm::ivec2(2.0f, 2.0f);

  // create a piece of worldspace text
  {
    const std::string label = "v0.0.5  Feedback? Discord @turtwiggy or X @Wiggy_dev";
    const ImVec2 xy = ImGui::CalcTextSize(label.c_str());
    const float padding = 10;

    const auto e = create_transform(r);
    auto& ui = r.emplace<WorldspaceTextComponent>(e);
    ui.text = label;
    set_position(r, e, { +half_wh.x + (xy.x / 2.0f) + padding, half_wh.y - xy.y - 4 });
    set_size(r, e, { 0, 0 });
  };

  // create a player for an interactive menu
  const auto player_e = create_gameplay(r, EntityType::actor_spaceship, { half_wh.x, 0 });
  r.emplace<PlayerComponent>(player_e);
  r.emplace<InputComponent>(player_e);
  r.emplace<KeyboardComponent>(player_e);
  r.emplace<DefaultColour>(player_e, engine::SRGBColour{ 255, 255, 117, 1.0f });
  add_particles(r, player_e);

  // r.emplace<PlayerComponent>(player_e);
  auto& player_thrust = r.emplace<MovementAsteroidsComponent>(player_e);
  player_thrust.able_to_change_thrust = false;
  player_thrust.able_to_change_dir = true;
  r.get<TransformComponent>(player_e).position.z = 1; // above particles
  r.emplace<CameraFollow>(player_e);
}

// scene idea:
// spawn a ship,
// boost the player ship for X seconds until it reaches the ship,
// move the already existing player to circle it.
// this is your "prepare your units phase"...
void
move_to_scene_overworld_revamped(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto half_wh = ri.viewport_size_render_at / glm::ivec2(2.0f, 2.0f);

  const auto player_e = get_first<PlayerComponent>(r);
  auto& player_t = r.get<TransformComponent>(player_e);

  // remove player control
  r.remove<InputComponent>(player_e);
  r.remove<MovementAsteroidsComponent>(player_e);

  // create an enemy ship off-screen
  const glm::vec2 enemy_pos = { player_t.position.x + half_wh.x * 2, player_t.position.y };
  const auto enemy_e = create_gameplay(r, EntityType::actor_spaceship, enemy_pos);
  const auto enemy_col = engine::SRGBColour{ 1.0f, 0.0f, 0.0f, 1.0f };
  r.get<TransformComponent>(enemy_e).position.z = 1; // above particles
  r.emplace<EnemyComponent>(enemy_e);
  r.emplace<DefaultColour>(enemy_e, enemy_col);
  set_colour(r, enemy_e, enemy_col);
  add_particles(r, enemy_e);

  // boost the player's ship until it reaches the enemy...
  r.emplace<DynamicTargetComponent>(player_e, enemy_e);
  r.emplace<ApplyForceToDynamicTarget>(player_e, 300.0f);

  // when within range...
  // start circling in this distance...

  constexpr int d2 = 200 * 200;
  DistanceCheckComponent distance_c;
  distance_c.d2 = d2;
  distance_c.e0 = player_e;
  distance_c.e1 = enemy_e;
  distance_c.action = [](entt::registry& r, const DistanceCheckComponent& info) {
    const auto player_e = get_first<PlayerComponent>(r);
    const auto enemy_e = get_first<EnemyComponent>(r);

    // remove target from player
    r.remove<ApplyForceToDynamicTarget>(player_e);
    r.remove<DynamicTargetComponent>(player_e);

    // give back movement control (to player)
    r.emplace<InputComponent>(player_e);
    r.emplace<MovementAsteroidsComponent>(player_e);

    // give the player some UI
    SpaceLabelComponent player_label;
    player_label.text = "You";
    player_label.ui_colour = engine::SRGBColour(0.3f, 1.0f, 0.3f, 1.0f);
    r.emplace<SpaceLabelComponent>(player_e, player_label);

    SpaceLabelComponent enemy_label;
    enemy_label.text = "Hostile Hauler";
    enemy_label.ui_colour = engine::SRGBColour{ 1.0f, 0.3f, 0.3f, 1.0f };
    r.emplace<SpaceLabelComponent>(enemy_e, enemy_label);

    // get the enemy ship to orbit your ship
    r.emplace<DynamicTargetComponent>(enemy_e, player_e);
    r.emplace<ApplyForceToDynamicTarget>(enemy_e);

    // shoot star-wars lasers at eachother
    // create_empty<SINGLE_OverworldFakeFight>(r);

    // get the enemy to quip
    RequestQuip quip_req;
    quip_req.type = QuipType::BEGIN_ENCOUNTER;
    quip_req.quipp_e = enemy_e;
    quip_req.seconds_to_quip = 6.0f;
    create_empty<RequestQuip>(r, quip_req);
  };
  create_empty<DistanceCheckComponent>(r, distance_c);
}

void
move_to_scene_additive(entt::registry& r, const Scene& s)
{
  stop_all_audio(r);

  if (s == Scene::overworld_revamped)
    move_to_scene_overworld_revamped(r);

  const auto scene_name = std::string(magic_enum::enum_name(s));
  fmt::println("additive scene. scene set to: {}", scene_name);

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

void
move_to_scene_start(entt::registry& r, const Scene& s, const bool load_saved)
{
  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());

  const auto& actors = r.view<EntityTypeComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(actors.begin(), actors.end());

  const auto& ui = r.view<WorldspaceTextComponent>();
  r.destroy(ui.begin(), ui.end());

  // store one physics world...
  static b2World* world = new b2World(b2Vec2(0.0f, 0.0f));

  // cleanup physics world...
  {
    static bool needs_deleting = false;
    if (needs_deleting) {
      b2Joint* joint = world->GetJointList();
      while (joint) {
        b2Joint* j = joint;
        joint = joint->GetNext();
        world->DestroyJoint(j);
      }
      b2Body* body = world->GetBodyList();
      while (body) {
        b2Body* b = body;
        body = body->GetNext();
        world->DestroyBody(b);
      }
    }
    needs_deleting = true;
  }
  destroy_first_and_create<SINGLE_Physics>(r, SINGLE_Physics{ world });

  destroy_first_and_create<SINGLETON_CurrentScene>(r);
  destroy_first_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_first_and_create<SINGLETON_GameStateComponent>(r);
  destroy_first_and_create<SINGLETON_GameOver>(r);
  destroy_first_and_create<SINGLETON_InputComponent>(r);
  destroy_first_and_create<SINGLE_ScreenshakeComponent>(r);

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
  destroy_first<SINGLE_OverworldFakeFight>(r);
  // destroy_first<SINGLE_MainMenuUI>(r);

  if (s != Scene::dungeon_designer)
    destroy_first<OverworldToDungeonInfo>(r);

  // systems that havent been destroyed...
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto& colours = get_first_component<SINGLE_ColoursInfo>(r);
  // const auto& audio = get_first_component<SINGLETON_AudioComponent>(r);

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
    const auto e = create_transform(r);
    const auto tex_unit = search_for_texture_unit_by_texture_path(ri, "blueberry").value();
    set_sprite_custom(r, e, "STUDIO_LOGO", tex_unit.unit);
    set_size(r, e, { 512, 512 });
    set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::menu)
    move_to_scene_menu(r);

  const auto get_seed_from_systemtime = []() -> time_t {
    auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
  };

  if (s == Scene::dungeon_designer) {
    // r.emplace_or_replace<CameraFreeMove>(get_first<OrthographicCamera>(r));
    destroy_first_and_create<SINGLE_CombatState>(r);
    destroy_first_and_create<SINGLE_EventConsoleLogComponent>(r);
    destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);
    // destroy_first_and_create<Effect_DoBloom>(r);
    destroy_first_and_create<Effect_GridComponent>(r);
    // create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "COMBAT_01" });

    if (get_first<OverworldToDungeonInfo>(r) == entt::null) {
      fmt::println("OverworldToDungeonInfo is null; assuming launch from standalone");
      OverworldToDungeonInfo info;
      info.placeholder = true;
      destroy_first_and_create<OverworldToDungeonInfo>(r, info);
    }

    // TEMP: add info in the event console on how to play.
    auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);
    evts.events.push_back("Press E to access inventory.");
    evts.events.push_back("Press 1 to select move action.");
    evts.events.push_back("Press 2 to select shoot action.");
    evts.events.push_back("Right click to perform action.");

    // create a cursor
    const auto cursor_e = create_gameplay(r, EntityType::cursor, { 0, 0 });
    set_size(r, cursor_e, { 0, 0 });

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_transform(r);
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled
  }

  if (s == Scene::turnbasedcombat) {
    r.emplace_or_replace<CameraFreeMove>(get_first<OrthographicCamera>(r));
    destroy_first_and_create<SINGLE_CombatState>(r);
    destroy_first_and_create<SINGLE_EventConsoleLogComponent>(r);
    destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);
    // destroy_first_and_create<Effect_DoBloom>(r);
    // create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "COMBAT_01" });

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
    const auto cursor_e = create_gameplay(r, EntityType::cursor, { 0, 0 });
    set_size(r, cursor_e, { 0, 0 });

    // make the entire debug space a room
    const int room_width = map_width / map_c.tilesize;
    const int room_height = map_height / map_c.tilesize;
    Room room;
    room.tl = { 0, 0 };
    room.aabb.center = { room.tl.x + (room_width / 2), room.tl.y + (room_height / 2) };
    room.aabb.size = { room_width, room_height };
    create_empty<Room>(r, room);

    DungeonGenerationResults result;
    result.rooms.push_back(room);
    std::vector<int> wall_or_floors(map_c.xmax * map_c.ymax, 0); // 0: everything as floor
    result.wall_or_floors = wall_or_floors;
    create_empty<DungeonGenerationResults>(r, result);

    // check if you started combat via a collison in e overworld
    if (get_first<OverworldToDungeonInfo>(r) != entt::null) {
      // const auto& data = get_first_component<OverworldToDungeonInfo>(r);
      // enemies = data.patrol_that_you_hit.strength;
    }

    int players = 1;
    int enemies = 1;

    // create player team
    for (int i = 0; i < players; i++) {
      const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
      const auto pos = glm::ivec2{ map_c.tilesize * 2, (i + 1) * (map_c.tilesize * 2) } + offset;

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::player;
      create_combat_entity(r, desc);
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

      const auto src_pos = get_position(r, last_spawned_e);
      const auto grid_pos = engine::grid::worldspace_to_grid_space(src_pos, map.tilesize);
      const int src_idx = engine::grid::grid_position_to_index(grid_pos, map.xmax);

      const int idx_next = get_lowest_cost_neighbour(r, map, src_idx, last_spawned_e);
      auto pos = engine::grid::index_to_world_position(idx_next, map.xmax, map.ymax, map.tilesize);
      pos += glm::ivec2{ map_c.tilesize / 2, map_c.tilesize / 2 }; // center

      CombatEntityDescription desc;
      desc.position = pos;
      desc.team = AvailableTeams::enemy;
      last_spawned_e = create_combat_entity(r, desc);
    }

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_gameplay(r, EntityType::empty_with_transform, { 0, 0 });
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled

    // Create a destructable barrel
    auto& map = get_first_component<MapComponent>(r);

    const auto gridpos = glm::ivec2{ 4, 4 };
    const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(gridpos, map_c.tilesize);
    const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
    const glm::ivec2 pos = worldspace + offset;

    const entt::entity barrel_e = create_gameplay(r, EntityType::actor_dungeon, pos);

    std::vector<entt::entity>& ents = map.map[engine::grid::grid_position_to_index(gridpos, map.xmax)];
    ents.push_back(barrel_e);
  }

  if (s == Scene::minigame_bamboo)
    create_empty<SINGLE_MinigameBamboo>(r);

  if (s == Scene::fov_tests) {
    engine::RandomState rnd;
    destroy_first_and_create<SINGLE_CombatState>(r);
    destroy_first_and_create<SINGLE_EventConsoleLogComponent>(r);
    destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);
    destroy_first_and_create<CameraFreeMove>(r);

    int map_width = 600;
    int map_height = 600;
    MapComponent map_c;
    map_c.tilesize = 50;
    map_c.xmax = map_width / map_c.tilesize;
    map_c.ymax = map_height / map_c.tilesize;
    map_c.map.resize(map_c.xmax * map_c.ymax);

    // Edge edge;
    // edge.cell_a = { 0, 0 };
    // edge.cell_b = { 0, 1 };
    // map_c.edges.push_back(edge);

    create_empty<MapComponent>(r, map_c);
    create_empty<Effect_GridComponent>(r, Effect_GridComponent{ map_c.tilesize });

    // create a cursor
    const auto cursor_e = create_gameplay(r, EntityType::cursor, { 0, 0 });
    set_size(r, cursor_e, { 0, 0 });

    // make the entire debug space a room
    const int room_width = map_width / map_c.tilesize;
    const int room_height = map_height / map_c.tilesize;
    Room room;
    room.tl = { 0, 0 };
    room.aabb.center = { room.tl.x + (room_width / 2), room.tl.y + (room_height / 2) };
    room.aabb.size = { room_width, room_height };
    create_empty<Room>(r, room);

    DungeonGenerationResults result;
    result.rooms.push_back(room);
    std::vector<int> wall_or_floors(map_c.xmax * map_c.ymax, 0); // 0: everything as floor
    result.wall_or_floors = wall_or_floors;
    create_empty<DungeonGenerationResults>(r, result);

    // DUPLICATE CODE: generate floor tiles
    result.floor_tiles.resize(result.wall_or_floors.size(), entt::null);
    for (int xy = 0; xy < result.wall_or_floors.size(); xy++) {
      if (result.wall_or_floors[xy] == 0) {
        const auto gridpos = engine::grid::index_to_grid_position(xy, map_c.xmax, map_c.ymax);
        const auto floor_e = create_transform(r);
        const glm::ivec2 worldspace = engine::grid::grid_space_to_world_space(gridpos, map_c.tilesize);
        const glm::ivec2 offset = { map_c.tilesize / 2.0f, map_c.tilesize / 2.0f };
        const glm::ivec2 pos = worldspace + offset;
        set_position(r, floor_e, pos);
        set_size(r, floor_e, { map_c.tilesize, map_c.tilesize });
        float rnd_col = engine::rand_det_s(rnd.rng, 0.3, 0.35f);
        set_colour(r, floor_e, { rnd_col, rnd_col, rnd_col, 1.0f });
        set_z_index(r, floor_e, -1);

        result.floor_tiles[engine::grid::grid_position_to_index(gridpos, map_c.xmax)] = floor_e;
      }
    }

    // DUPLICATE CODE: add info in the event console on how to play.
    auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);
    evts.events.push_back("Press E to access inventory.");
    evts.events.push_back("Press 1 to select move action.");
    evts.events.push_back("Press 2 to select shoot action.");
    evts.events.push_back("Right click to perform action.");

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_transform(r);
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled

    const glm::ivec2 offset = { map_c.tilesize / 2, map_c.tilesize / 2 };
    const auto pos = glm::ivec2{ map_c.tilesize * 2, (1) * (map_c.tilesize * 2) } + offset;
    CombatEntityDescription desc;
    desc.position = pos;
    desc.team = AvailableTeams::player;
    create_combat_entity(r, desc);
  }

  const auto scene_name = std::string(magic_enum::enum_name(s));
  fmt::println("setting scene to: {}", scene_name);

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d