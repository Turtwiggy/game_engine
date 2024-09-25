#include "scene_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "components.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/system_distance_check/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_lootbag/components.hpp"
#include "modules/ui_overworld_boardship/components.hpp"
#include "modules/ui_overworld_shiplabel/components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include <magic_enum.hpp>

namespace game2d {

void
create_player_if_not_in_scene(entt::registry& r)
{
  auto player_e = get_first<PlayerComponent>(r);
  if (player_e != entt::null)
    return;

  const auto pos = glm::vec2{ 0, 0 };
  auto e = spawn_mob(r, "spaceship_player", pos);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<PlayerComponent>(e);
  r.emplace<InputComponent>(e);
  r.emplace<KeyboardComponent>(e);
  auto& player_thrust = r.emplace<MovementAsteroidsComponent>(e);
  player_thrust.able_to_change_thrust = false;
  player_thrust.able_to_change_dir = true;
  set_size(r, e, { 16, 16 });

  spawn_particle_emitter(r, "anything", { 0, 0 }, e);
};

void
move_to_scene_start(entt::registry& r, const Scene& s)
{
  const auto scene_name = std::string(magic_enum::enum_name(s));
  fmt::println("going to scene: {}", scene_name);

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
  create_empty<SINGLE_UIInventoryState>(r);
  create_empty<SINGLE_UI_Lootbag>(r);

  // The first and only transform should be the camera
  const auto camera_e = get_first<TransformComponent>(r);
  if (auto* can_move = r.try_get<CameraFreeMove>(camera_e))
    r.remove<CameraFreeMove>(camera_e); // reset to default
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
    r.remove<InputComponent>(player_e);
    r.remove<KeyboardComponent>(player_e);
    r.remove<MovementAsteroidsComponent>(player_e);

    // create an enemy ship off-screen
    const auto& player_pos = get_position(r, player_e);
    const auto half_wh = ri.viewport_size_render_at / glm::ivec2(2.0f, 2.0f);
    const auto pos = glm::vec2{ player_pos.x + half_wh.x * 2, player_pos.y };
    const auto enemy_e = spawn_mob(r, "spaceship_enemy", pos);
    r.emplace<TeamComponent>(enemy_e, TeamComponent{ AvailableTeams::enemy });
    r.emplace<EnemyComponent>(enemy_e);
    set_size(r, enemy_e, { 16, 16 });

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
      r.emplace<InputComponent>(player_e);
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
  fmt::println("additive scene. scene set to: {}", scene_name);

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d