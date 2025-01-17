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
#include "modules/effects_outline/outline_components.hpp"
#include "modules/event_coll_bullet_enemy/event_coll_bullet_enemy_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_spawner/spawner_components.hpp"
#include "modules/system_spawner/spawner_helpers.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_survive_level_up/ui_survive_level_up_components.hpp"
#include "modules/ui_survive_timer/ui_survive_timer_components.hpp"
#include "modules/ui_units/ui_units_helpers.hpp"

#include <magic_enum.hpp>

namespace game2d {

entt::entity
spawn_player(entt::registry& r, std::string key, glm::ivec2 pos, int num)
{
  glm::vec2 dinghy_size = { 33, 18 };
  glm::vec2 rhib_size = { 55, 30 };
  glm::vec2 weapon_size = { 5, 10 };

  const auto e = spawn(r, key);
  give_life(r, e, pos, dinghy_size);
  r.emplace<PlayerComponent>(e, num);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<SpriteOutline>(e);
  r.emplace<MovementDirectComponent>(e);
  r.emplace<SetTransformRotationBasedOnPhysicsBody>(e);
  r.get<PhysicsBodyComponent>(e).base_speed = 100.0f;
  spawn_particle_emitter(r, "anything", { 0, 1 }, e);
  r.emplace<BulletDamage>(e); // probably shouldnt be on body

  // player fixture
  auto player_fixture_e = get_fixture_by_tag(r, e, "player");
  r.emplace<PlayerFixtureComponent>(player_fixture_e);

  // xp_zone fixture
  auto fixture_e = get_fixture_by_tag(r, e, "xp_zone");
  r.emplace<XpZoneComponent>(fixture_e);

  // player weapon
  const auto wep_e = spawn(r, "boat_default_weapon");
  give_life(r, wep_e, get_position(r, e), weapon_size);
  r.emplace<TeamComponent>(wep_e, TeamComponent{ AvailableTeams::player });
  r.emplace<HasWeaponComponent>(e, HasWeaponComponent{ wep_e }); // parent <=> child
  r.emplace<HasParentComponent>(wep_e, HasParentComponent{ e }); // child <=> parent
  r.emplace<WeaponComponent>(wep_e);
  r.emplace<CooldownComponent>(wep_e, CooldownComponent{ 0.5f, 0.5f });
  set_z_index(r, wep_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);
  set_colour(r, wep_e, { 1.0f, 1.0f, 1.0f, 1.0f });

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

    // TEMP: clear assigned controller handles?
    // this is annoying for players
    auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
    steam_c.assigned_handles.clear();

    // auto e = create_empty<TransformComponent>(r);
    // r.emplace<SpriteComponent>(e);
    // set_sprite(r, e, "STUDIO_TEXT_LOGO");
    // set_size(r, e, { 512, 256 });
    // set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::survive) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "GAME_01", true });
    create_empty<Effect_GridComponent>(r);
    create_empty<SINGLE_XpComponent>(r);
    create_empty<SINGLE_LevelUpUI>(r);

    // players
    const auto p1 = spawn_player(r, "actor_player", { 0, 0 }, 0);
    // const auto p2 = spawn_player(r, "actor_player", { 16, 0 }, 1);
    // const auto p3 = spawn_player(r, "actor_player", { 0, 16 }, 2);
    // const auto p4 = spawn_player(r, "actor_player", { 16, 16 }, 3);

    // inputs => players
    r.emplace<KeyboardComponent>(p1);
    r.emplace<SteamControllerComponent>(p1);
    // r.emplace<SteamControllerComponent>(p3);

    // The survive timer that various spawners read from
    float seconds = 20 * 60;
    const auto survive_timer_e = create_empty<CooldownComponent>(r, CooldownComponent{ seconds, seconds });
    r.emplace<SurviveTimerComponent>(survive_timer_e);

    const auto spawner_1_e = create_empty<CooldownComponent>(r);
    r.emplace<EnemySpawnData>(spawner_1_e, exploder_data());

    const auto spawner_2_e = create_empty<CooldownComponent>(r);
    r.emplace<EnemySpawnData>(spawner_2_e, melee_enemy_1());

    const auto spawner_3_e = create_empty<CooldownComponent>(r);
    r.emplace<EnemySpawnData>(spawner_3_e, melee_enemy_2());

    const auto spawner_4_e = create_empty<CooldownComponent>(r);
    r.emplace<EnemySpawnData>(spawner_4_e, projectile_enemy());

    // something random
    {
      auto e = create_empty<TransformComponent>(r);
      r.emplace<SpriteComponent>(e);
      set_sprite(r, e, "random_decal");
      set_size(r, e, { 64, 64 });
      set_position(r, e, { -32, -32 });
      set_z_index(r, e, ZLayer::BACKGROUND);
    }
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