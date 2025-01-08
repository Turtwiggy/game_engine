#include "scene_helpers.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
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
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_spawner/spawner_components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "modules/ui_lootbag/ui_lootbag_components.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_survive_timer/ui_survive_timer_components.hpp"
#include "modules/ui_units/ui_units_helpers.hpp"

#include <magic_enum.hpp>

namespace game2d {

entt::entity
spawn_player(entt::registry& r, std::string key, glm::ivec2 pos, int num)
{
  const auto e = spawn(r, key);
  give_life(r, e, pos, { 32, 32 });
  r.emplace<PlayerComponent>(e, num);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<MovementDirectComponent>(e);
  r.emplace<SetTransformRotationBasedOnPhysicsBody>(e);
  r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;
  spawn_particle_emitter(r, "anything", { 0, 1 }, e);

  // player weapon
  const auto wep_e = spawn(r, "shotgun");
  give_life(r, wep_e, get_position(r, e), { 16, 16 });
  r.emplace<HasWeaponComponent>(e, HasWeaponComponent{ wep_e }); // parent <=> child
  r.emplace<HasParentComponent>(wep_e, HasParentComponent{ e }); // child <=> parent
  r.emplace<TeamComponent>(wep_e, TeamComponent{ AvailableTeams::player });
  r.emplace<WeaponComponent>(wep_e);
  r.emplace<CooldownComponent>(wep_e, CooldownComponent{ 0.5f, 0.5f });
  set_z_index(r, wep_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);

  return e;
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
  begin_frame_sprite(r); // initialize cached for sprites

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

    // load player's saved units
    const auto units = load_units(r);
    std::for_each(units.begin(), units.end(), [&r](const auto& u) { add_unit_to_entt(r, u); });

    // auto e = create_empty<TransformComponent>(r);
    // r.emplace<SpriteComponent>(e);
    // set_sprite(r, e, "STUDIO_TEXT_LOGO");
    // set_size(r, e, { 512, 256 });
    // set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::survive) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "GAME_01", true });

    // default player (keyboard)
    const auto p1 = spawn_player(r, "actor_player_1", { 0, 0 }, 0);
    const auto p2 = spawn_player(r, "actor_player_2", { 16, 0 }, 1);
    const auto p3 = spawn_player(r, "actor_player_3", { 0, 16 }, 2);
    const auto p4 = spawn_player(r, "actor_player_4", { 16, 16 }, 3);

    // inputs
    r.emplace<KeyboardComponent>(p1);
    r.emplace<ControllerComponent>(p2);

    // spawn endless enemies
    const auto spawner_e = create_empty<SpawnerComponent>(r);
    r.emplace<CooldownComponent>(spawner_e, 3.0f, 0.0f);

    // a timer
    float minutes_20 = 20 * 60;
    const auto timer_e = create_empty<CooldownComponent>(r, CooldownComponent{ minutes_20, minutes_20 });
    r.emplace<SurviveTimerComponent>(timer_e);
  }

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
};

void
move_to_scene_additive(entt::registry& r, const Scene& s)
{
  audio::sdl_mixer::stop_all_audio(r);

  //
  //
  //

  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("additive scene. scene set to: {}", scene_name).c_str());

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
}

} // namespace game2d