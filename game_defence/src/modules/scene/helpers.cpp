#include "modules/scene/helpers.hpp"

#include "actors/actors.hpp"
#include "actors/helpers.hpp"
#include "audio/components.hpp"
#include "audio/helpers/sdl_mixer.hpp"
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "modules/debug_map/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/system_distance_check/components.hpp"
#include "modules/system_minigame_bamboo/components.hpp"
#include "modules/system_overworld_fake_fight/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_combat_turnbased/components.hpp"
#include "modules/ui_event_console/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_overworld_launch_crew/components.hpp"
#include "modules/ui_overworld_ship_label/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_selected/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/helpers.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"
#include <box2d/b2_math.h>
#include <nlohmann/json.hpp>

#include <fmt/core.h>
#include <string>

namespace game2d {
using namespace audio::sdl_mixer;
using namespace std::literals;
using json = nlohmann::json;

entt::entity
add_weapon_shotgun(entt::registry& r, const entt::entity& e)
{
  DataWeaponShotgun desc;
  desc.pos = get_position(r, e);
  desc.team = r.get<TeamComponent>(e).team;
  desc.parent = get_first<PlayerComponent>(r);
  const auto weapon_e = Factory_DataWeaponShotgun::create(r, desc);
  // add_components(r, e, desc);

  // link player&weapon
  HasWeaponComponent has_weapon;
  has_weapon.instance = weapon_e;
  r.emplace<HasWeaponComponent>(e, has_weapon);

  return weapon_e;
};

void
move_to_scene_menu(entt::registry& r)
{
  destroy_first_and_create<SINGLE_MainMenuUI>(r);
  destroy_first_and_create<Effect_GridComponent>(r);
  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01", true });
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  // Load randoms name file
  // const auto path = "./assets/config/random_names.json";
  // std::ifstream f(path);
  // json data = json::parse(f);
  // const auto names = data["names"]; // list of names

  // choose X random names, display them on the menu
  // static engine::RandomState rnd;
  // for (size_t i = 0; i < 4; i++) {
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

  // create a player for an interactive menu
  DataSpaceShipActor desc;
  desc.pos = { half_wh.x, 0 };
  desc.colour = { 255, 255, 117, 1.0f };
  desc.team = AvailableTeams::player;
  const auto player_e = Factory_DataSpaceShipActor::create(r, desc);
  r.emplace<CameraFollow>(player_e);
};

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

  DataSpaceShipActor desc;
  desc.pos = { player_t.position.x + half_wh.x * 2, player_t.position.y };
  desc.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
  desc.team = AvailableTeams::enemy;
  const auto enemy_e = Factory_DataSpaceShipActor::create(r, desc);

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
    enemy_label.text = "Small Ship";
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

    // allow user to board the ship
    create_empty<RequestShowBoardShipButton>(r, RequestShowBoardShipButton{ true });
  };
  create_empty<DistanceCheckComponent>(r, distance_c);
};

void
move_to_scene_additive(entt::registry& r, const Scene& s)
{
  stop_all_audio(r);

  if (s == Scene::overworld_revamped) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "GAME_01" });
    move_to_scene_overworld_revamped(r);
  }

  const auto scene_name = std::string(magic_enum::enum_name(s));
  fmt::println("additive scene. scene set to: {}", scene_name);

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
};

void
move_to_scene_start(entt::registry& r, const Scene& s, const bool load_saved)
{
  const auto& transforms = r.view<TransformComponent>(entt::exclude<OrthographicCamera>);
  r.destroy(transforms.begin(), transforms.end());
  const auto& ui = r.view<WorldspaceTextComponent>();
  r.destroy(ui.begin(), ui.end());

  // anything created with create_empty?
  const auto& rooms = r.view<Room>();
  r.destroy(rooms.begin(), rooms.end());
  const auto& tunnels = r.view<Tunnel>();
  r.destroy(tunnels.begin(), tunnels.end());
  const auto& paths = r.view<PathfindComponent>();
  r.destroy(paths.begin(), paths.end());
  const auto& camera_move = r.view<CameraFreeMove>();
  r.destroy(camera_move.begin(), camera_move.end());
  const auto& invs = r.view<InventorySlotComponent>();
  r.destroy(invs.begin(), invs.end());

  emplace_or_replace_physics_world(r);

  destroy_first_and_create<SINGLETON_CurrentScene>(r);
  destroy_first_and_create<SINGLETON_EntityBinComponent>(r);
  destroy_first_and_create<SINGLETON_GameStateComponent>(r);
  destroy_first_and_create<SINGLETON_GameOver>(r);
  destroy_first_and_create<SINGLETON_InputComponent>(r);
  destroy_first_and_create<SINGLE_ScreenshakeComponent>(r);
  destroy_first_and_create<SINGLE_EventConsoleLogComponent>(r);
  destroy_first_and_create<SINGLE_UIInventoryState>(r);

  destroy_first<SINGLE_SelectedUI>(r);
  destroy_first<SINGLE_TurnBasedCombatInfo>(r);
  destroy_first<SINGLE_CombatState>(r);
  destroy_first<MapComponent>(r);
  destroy_first<DebugMapComponent>(r);
  destroy_first<Effect_GridComponent>(r);
  destroy_first<Effect_DoBloom>(r);
  destroy_first<SINGLE_MinigameBamboo>(r);
  destroy_first<SINGLE_TurnBasedCombatInfo>(r);
  destroy_first<DungeonGenerationResults>(r);
  destroy_first<SINGLE_OverworldFakeFight>(r);
  // destroy_first<SINGLE_MainMenuUI>(r);

  if (s != Scene::dungeon_designer)
    destroy_first<OverworldToDungeonInfo>(r);

  // systems that havent been destroyed...
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  // const auto& anims = get_first_component<SINGLE_Animations>(r);
  // const auto& colours = get_first_component<SINGLE_ColoursInfo>(r);
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

  if (s == Scene::dungeon_designer) {
    // r.emplace_or_replace<CameraFreeMove>(get_first<OrthographicCamera>(r));
    destroy_first_and_create<SINGLE_CombatState>(r);
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
    evts.events.push_back("gun_slot=>bomb. Hold left click to place.");
    evts.events.push_back("gun_slot=>shotgun ammo_slot=>ammo. Perform shoot action.");
    evts.events.push_back("Press Q to perform heal action.");
    evts.events.push_back("Press 1 to select move action.");
    evts.events.push_back("Press 2 to select shoot action.");
    evts.events.push_back("Right click to perform action.");

    // Debug object
    auto& info = get_first_component<SINGLE_TurnBasedCombatInfo>(r);
    info.action_cursor = create_transform(r);
    set_size(r, info.action_cursor, { 0, 0 }); // start disabled
  }

  if (s == Scene::minigame_bamboo)
    create_empty<SINGLE_MinigameBamboo>(r);

  const auto scene_name = std::string(magic_enum::enum_name(s));
  fmt::println("setting scene to: {}", scene_name);

  auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  scene.s = s; // done
};

} // namespace game2d