#include "scene_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/system_distance_check/components.hpp"
#include "modules/system_initiative/initiative_components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/system_select_unit/select_unit_components.hpp"
#include "modules/system_tutorial/tutorial_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_inventory/ui_inventory_helpers.hpp"
#include "modules/ui_lootbag/ui_lootbag_components.hpp"
#include "modules/ui_overworld_boardship/components.hpp"
#include "modules/ui_overworld_shiplabel/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_units/ui_units_helpers.hpp"

#include <magic_enum.hpp>

namespace game2d {

void
create_player_if_not_in_scene(entt::registry& r)
{
  const auto player_e = get_first<PlayerComponent>(r);
  if (player_e != entt::null)
    return;

  const auto pos = glm::vec2{ 0, 0 };
  auto e = spawn_mob(r, "spaceship_player");
  give_life(r, e, pos, { 16, 16 });

  // r.emplace<CameraLerpToTarget>(e);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<PlayerComponent>(e);
  r.emplace<KeyboardComponent>(e);
  r.emplace<SelectedComponent>(e);
  auto& player_thrust = r.emplace<MovementAsteroidsComponent>(e);
  player_thrust.able_to_change_thrust = false;
  player_thrust.able_to_change_dir = true;
  set_dir(r, e, engine::normalize_safe(engine::angle_radians_to_direction(-30 * engine::Deg2Rad)));

  spawn_particle_emitter(r, "anything", { 0, 1 }, e);
};

void
setup_tutorial_scene(entt::registry& r, bool in_tutorial_scene)
{
  if (!in_tutorial_scene)
    return;

  create_empty<CameraFreeMove>(r);
  create_empty<Effect_GridComponent>(r);
  create_empty<SINGLE_TutorialMetrics>(r);

  int tilesize = 32;
  destroy_first_and_create<MapComponent>(r);
  auto& map = get_first_component<MapComponent>(r);
  map.tilesize = tilesize;
  map.xmax = 10;
  map.ymax = 10;
  map.map.resize(map.xmax * map.ymax);

  auto grid_e = get_first<Effect_GridComponent>(r);
  if (grid_e != entt::null)
    get_first_component<Effect_GridComponent>(r).gridsize = tilesize;
  const auto map_e = get_first<MapComponent>(r);
  const auto& map_c = r.get<MapComponent>(map_e);
  const glm::vec2 map_center = { (map_c.xmax * map_c.tilesize) / 2.0f, (map_c.ymax * map_c.tilesize) / 2.0f };
  const glm::vec2 map_size = { map_c.xmax * map_c.tilesize, map_c.ymax * map_c.tilesize };

  // center the camera
  const auto camera_e = get_first<OrthographicCamera>(r);
  set_position(r, camera_e, map_center);
};

void
move_to_scene_start(entt::registry& r, const Scene& s)
{
  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("going to scene: {}", scene_name).c_str());

  for (const std::tuple<entt::entity>& ent_tuple : r.storage<entt::entity>().each()) {
    const auto& [e] = ent_tuple;
    if (const auto* p_c = r.try_get<Persistent>(e))
      continue;
    r.destroy(e);
  };

  // box2d to handle it's own cleanup
  emplace_or_replace_physics_world(r);

  // do not use create_persistent here. anything created
  // here should be expected to be removed between scenes
  create_empty<SINGLE_CurrentScene>(r);
  create_empty<SINGLE_EntityBinComponent>(r);
  create_empty<SINGLE_GameStateComponent>(r);
  create_empty<SINGLE_InputComponent>(r);
  create_empty<SINGLE_ScreenshakeComponent>(r);
  create_empty<SINGLE_UIInventoryState>(r);
  create_empty<SINGLE_UI_Lootbag>(r);
  create_empty<SINGLE_ImSprite>(r);

  // The first and only transform should be the camera
  const auto camera_e = get_first<OrthographicCamera>(r);
  r.get<TransformComponent>(camera_e).position = { 0, 0, 0 };
  r.get<TransformComponent>(camera_e).scale = { 0, 0, 0 };

  audio::sdl_mixer::stop_all_audio(r);

  if (s == Scene::splashscreen) {
    create_empty<SINGLE_SplashScreen>(r);

    auto e = create_empty<TransformComponent>(r);
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, "STUDIO_LOGO");
    set_size(r, e, { 512, 512 });
    set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::menu) {
    create_empty<SINGLE_MainMenuUI>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01", true });
    create_player_if_not_in_scene(r);

    // load player's saved units
    const auto units = load_units(r);
    std::for_each(units.begin(), units.end(), [&r](const auto& u) { add_unit_to_entt(r, u); });

    // auto e = create_empty<TransformComponent>(r);
    // r.emplace<SpriteComponent>(e);
    // set_sprite(r, e, "STUDIO_TEXT_LOGO");
    // set_size(r, e, { 512, 256 });
    // set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::dungeon_designer) {
    create_empty<CameraFreeMove>(r);
    create_empty<Effect_GridComponent>(r);
    // destroy_first_and_create<SINGLE_CombatState>(r);
    // destroy_first_and_create<SINGLE_TurnBasedCombatInfo>(r);

    // TEMP: add info in the event console on how to play.
    // auto& evts = get_first_component<SINGLE_EventConsoleLogComponent>(r);
    // evts.events.push_back("Press WASD to move.");
    // evts.events.push_back("Press E to open/close inventory.");
    // evts.events.push_back("Press R to open/close loot");
    // evts.events.push_back("Left click to perform item action.");
  }

  const std::vector<Scene> tutorial_scenes{ Scene::tutorial_shotgun_straight,
                                            Scene::tutorial_shotgun_diagonal,
                                            Scene::tutorial_knife_bleed,
                                            Scene::tutorial_hook_blackhole };
  bool in_tutorial_scene = std::find(tutorial_scenes.begin(), tutorial_scenes.end(), s) != tutorial_scenes.end();

  setup_tutorial_scene(r, in_tutorial_scene);

  if (s == Scene::tutorial_shotgun_straight) {
    const auto players_idxs = std::vector<int>{ 44 };
    const auto enemy_idxs = std::vector<int>{ 45 };
    const auto blackhole_idxs = std::vector<int>{ 46 };
    const auto players = spawn_n_players(r, players_idxs, { UnitType{} });
    const auto enemies = spawn_n_enemies(r, enemy_idxs, 1);
    const auto blackholes = spawn_n_blackhole(r, blackhole_idxs, 1);

    if (auto* debug_req = r.try_get<DebugBodyAndInventory>(players[0]))
      r.remove<DebugBodyAndInventory>(players[0]);
    spawn_inv_item(r, r.get<DefaultBody>(players[0]).body, 0, "shotgun");

    auto& tutorial_c = get_first_component<SINGLE_TutorialMetrics>(r);
    tutorial_c.max_turns = 1;
    tutorial_c.objective = "Destroy the enemy in one move.";
    tutorial_c.expected_enemies = 0;
  }
  if (s == Scene::tutorial_shotgun_diagonal) {
    const auto players_idxs = std::vector<int>{ 44 };
    const auto enemy_idxs = std::vector<int>{ 35, 36, 46, 55, 56 };
    const auto blackhole_idxs = std::vector<int>{ 26, 27, 47, 66, 67 };
    const auto players = spawn_n_players(r, players_idxs, { UnitType{} });
    const auto enemies = spawn_n_enemies(r, enemy_idxs, 5);
    const auto blackholes = spawn_n_blackhole(r, blackhole_idxs, 5);

    if (auto* debug_req = r.try_get<DebugBodyAndInventory>(players[0]))
      r.remove<DebugBodyAndInventory>(players[0]);
    spawn_inv_item(r, r.get<DefaultBody>(players[0]).body, 0, "shotgun");

    auto& tutorial_c = get_first_component<SINGLE_TutorialMetrics>(r);
    tutorial_c.max_turns = 1;
    tutorial_c.objective = "Destroy the enemies in one move.";
    tutorial_c.expected_enemies = 0;
  }
  if (s == Scene::tutorial_knife_bleed) {
    const auto players_idxs = std::vector<int>{ 44 };
    const auto enemy_idxs = std::vector<int>{ 45 };
    const auto blackhole_idxs = std::vector<int>{ 46 };
    const auto players = spawn_n_players(r, players_idxs, { UnitType{} });
    const auto enemies = spawn_n_enemies(r, enemy_idxs, 1);
    const auto blackholes = spawn_n_blackhole(r, blackhole_idxs, 1);

    if (auto* debug_req = r.try_get<DebugBodyAndInventory>(players[0]))
      r.remove<DebugBodyAndInventory>(players[0]);
    spawn_inv_item(r, r.get<DefaultBody>(players[0]).body, 0, "scrap_knife");

    auto& tutorial_c = get_first_component<SINGLE_TutorialMetrics>(r);
    tutorial_c.max_turns = 10;
    tutorial_c.objective = "Kill the enemy with bleed damage.";
    tutorial_c.expected_enemies = 0;
  }
  if (s == Scene::tutorial_hook_blackhole) {
    const auto players_idxs = std::vector<int>{ 44 };
    const auto enemy_idxs = std::vector<int>{ 48, 46 };
    const auto blackhole_idxs = std::vector<int>{ 47, 45 };
    const auto players = spawn_n_players(r, players_idxs, { UnitType{} });
    const auto enemies = spawn_n_enemies(r, enemy_idxs, 2);
    const auto blackholes = spawn_n_blackhole(r, blackhole_idxs, 2);

    if (auto* debug_req = r.try_get<DebugBodyAndInventory>(players[0]))
      r.remove<DebugBodyAndInventory>(players[0]);
    spawn_inv_item(r, r.get<DefaultBody>(players[0]).body, 0, "hook");

    auto& tutorial_c = get_first_component<SINGLE_TutorialMetrics>(r);
    tutorial_c.max_turns = 1;
    tutorial_c.objective = "Kill the enemy using the environment.";
    tutorial_c.expected_enemies = 0;
  }

  if (in_tutorial_scene) {
    // Set the first unit as the active unit
    auto initiative_group = r.group<InitiativeComponent>();
    initiative_group.sort<InitiativeComponent>(
      [](const InitiativeComponent& a, const InitiativeComponent& b) { return a.initiative < b.initiative; });
    for (const auto& [e, c] : initiative_group.each()) {
      activate_unit(r, e);
      break;
    }
  }

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
};

void
move_to_scene_additive(entt::registry& r, const Scene& s)
{
  audio::sdl_mixer::stop_all_audio(r);

  // scene idea:
  // spawn a ship,
  // boost the player ship for X seconds until it reaches the ship,
  // move the already existing player to circle it.
  // this is your "prepare your units phase"...
  if (s == Scene::overworld) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    create_empty<SINGLE_UIBoardShip>(r);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "GAME_01" });
    create_player_if_not_in_scene(r);

    // remove player control
    const auto player_e = get_first<PlayerComponent>(r);
    r.remove<KeyboardComponent>(player_e);
    r.remove<MovementAsteroidsComponent>(player_e);

    // create an enemy ship off-screen
    const auto& player_pos = get_position(r, player_e);
    const auto half_wh = ri.viewport_size_render_at / glm::ivec2(2.0f, 2.0f);
    const auto pos = glm::vec2{ player_pos.x + half_wh.x * 2, player_pos.y };
    const auto enemy_e = spawn_mob(r, "spaceship_enemy");
    give_life(r, enemy_e, pos, { 16, 16 });
    r.emplace<TeamComponent>(enemy_e, TeamComponent{ AvailableTeams::enemy });
    r.emplace<EnemyComponent>(enemy_e);

    // boost the player's ship until it reaches the enemy...
    r.emplace<PhysicsDynamicTarget>(player_e, enemy_e);
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
      r.remove<PhysicsDynamicTarget>(player_e);

      // give back movement control (to player)
      r.emplace<KeyboardComponent>(player_e);
      r.emplace<MovementAsteroidsComponent>(player_e);

      // give the player some UI
      SpaceLabelComponent player_label;
      player_label.text = "You";
      player_label.ui_colour = engine::SRGBColour(0.3f, 1.0f, 0.3f, 1.0f);
      r.emplace<SpaceLabelComponent>(player_e, player_label);

      SpaceLabelComponent enemy_label;
      enemy_label.text = "Small Ship";
      enemy_label.ui_colour = engine::SRGBColour{ 1.0f, 0.3f, 0.3f, 1.0f };
      r.emplace<SpaceLabelComponent>(enemy_e, enemy_label);

      // get the enemy ship to orbit your ship
      r.emplace<PhysicsDynamicTarget>(enemy_e, player_e);
      r.emplace<ApplyForceToDynamicTarget>(enemy_e);

      // shoot star-wars lasers at eachother
      // create_empty<SINGLE_OverworldFakeFight>(r);

      // get the enemy to quip
      RequestQuip quip_req;
      quip_req.type = QuipType::BEGIN_ENCOUNTER;
      quip_req.quipp_e = enemy_e;
      quip_req.seconds_to_quip = 6.0f;
      create_empty<RequestQuip>(r, quip_req);

      // allow user to board the ship
      create_empty<RequestShowBoardShipButton>(r, RequestShowBoardShipButton{ true });
    };
    create_empty<DistanceCheckComponent>(r, distance_c);
  };

  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("additive scene. scene set to: {}", scene_name).c_str());

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d