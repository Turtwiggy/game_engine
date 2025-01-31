#include "scene_helpers.hpp"

#include "components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/effects_outline/outline_components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/components.hpp"
#include "modules/screenshake/components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/system_autofire/autofire_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_hulls/hulls_components.hpp"
#include "modules/system_hulls/hulls_helpers.hpp"
#include "modules/system_manualfire/manualfire_components.hpp"
#include "modules/system_spawner/spawner_components.hpp"
#include "modules/system_spawner/spawner_helpers.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_scene_main_menu/components.hpp"
#include "modules/ui_scene_select/scene_select_components.hpp"
#include "modules/ui_survive_level_up/ui_survive_level_up_components.hpp"
#include "modules/ui_survive_timer/ui_survive_timer_components.hpp"
#include "modules/ui_units/ui_units_helpers.hpp"

#include <magic_enum.hpp>

namespace game2d {

glm::vec2 weapon_size = { 5, 10 };

entt::entity
spawn_weapon(entt::registry& r, entt::entity e, const HardpointData& data)
{
  const auto wep_e = spawn(r, "boat_default_weapon");
  give_life(r, wep_e, get_position(r, e), weapon_size);
  r.emplace<TeamComponent>(wep_e, TeamComponent{ AvailableTeams::player });
  r.emplace<HardpointComponent>(wep_e, HardpointComponent{ data });

  // parent <=> child
  auto& weapons_c = r.get_or_emplace<HasWeaponsComponent>(e);
  weapons_c.weapons.push_back(wep_e);

  // child <=> parent
  r.emplace<HasParentComponent>(wep_e, HasParentComponent{ e });

  // weapon stats
  float firerate = 0.5;
  r.emplace<WeaponComponent>(wep_e);
  r.emplace<WeaponProjectiles>(wep_e, WeaponProjectiles{ 1 });
  r.emplace<WeaponSpread>(wep_e, WeaponSpread{ 30 });
  r.emplace<WeaponFirerate>(wep_e, WeaponFirerate{ firerate });
  r.emplace<CooldownComponent>(wep_e, CooldownComponent{ firerate, firerate });

  // bullets that the weapon fires
  r.emplace<BulletDamage>(wep_e, 10);
  r.emplace<BulletPierce>(wep_e, 1);
  r.emplace<BulletSize>(wep_e, BulletSize{ { 6, 6 } });
  r.emplace<BulletSpeed>(wep_e, 250);
  r.emplace<BulletKnockback>(wep_e, 50);
  r.emplace<BulletBounce>(wep_e, 0); // no bounce by default

  set_z_index(r, wep_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);
  set_colour(r, wep_e, r.get<DefaultColour>(e).colour);
  return wep_e;
};

entt::entity
spawn_player(entt::registry& r, std::string key, glm::ivec2 pos, int num, std::string hull_key)
{
  const auto e = spawn(r, key);

  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
  ShipHullData hull = get_hull(hulls_c, hull_key).value();

  // ShipHullComponent hull_c;
  // hull_c.data = hull;
  // r.emplace<ShipHullComponent>(e, hull_c);

  const auto size = glm::vec2{ hull.width, hull.height };

  give_life(r, e, pos, size);
  r.emplace<PlayerComponent>(e, num);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<SpriteOutline>(e);
  r.emplace<MovementDirectComponent>(e);
  r.emplace<SetTransformRotationBasedOnPhysicsBody>(e);
  spawn_particle_emitter(r, "anything", { 0, 1 }, e);

  // Apply some drag, bro
  r.get<PhysicsBodyComponent>(e).body->SetLinearDamping(0.75);

  // TODO: come up with something better
  if (hull_key == "Dinghy")
    set_sprite(r, e, "hull_dinghy");
  if (hull_key == "RHIB")
    set_sprite(r, e, "hull_rhib");
  if (hull_key == "Constitution")
    set_sprite(r, e, "hull_constitution");
  if (hull_key == "PBR")
    set_sprite(r, e, "hull_pbr");
  if (hull_key == "Trimanan")
    set_sprite(r, e, "hull_trimanan");

  if (num == 0)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#cfc041")); // gold_yellow
  if (num == 1)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#da5bd6")); // magenta
  if (num == 2)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#00c420")); // green
  if (num == 3)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#0096ff")); // blue
  set_colour(r, e, r.get<DefaultColour>(e).colour);

  auto player_fixture_e = get_fixture_by_tag(r, e, "player");
  r.emplace<PlayerFixtureComponent>(player_fixture_e);
  r.emplace<HealthComponent>(player_fixture_e, 10, 10);
  // r.emplace<DefenceComponent>(player_fixture_e, 0);

  // xp_zone fixture
  auto fixture_e = get_fixture_by_tag(r, e, "xp_zone");
  r.emplace<XpZoneComponent>(fixture_e);

  // upgrades...
  r.emplace<StatModifierComponent>(e);

  // Spawn the weapons...
  for (auto& hardpoint_data : hull.hardpoints) {

    // HACK: overrode all arcs to 360 degrees. i.e. full coverage
    hardpoint_data.arc = 360;
    hardpoint_data.arc_mid = 0;

    auto weapon_e = spawn_weapon(r, e, hardpoint_data);
    r.emplace<AutofireComponent>(weapon_e);

    // break; // one weapon
  }

  // Spawn a manual weapon
  {
    HardpointComponent hardpoint_c;
    HardpointData hardpoint_data;
    hardpoint_data.key = "manual";
    hardpoint_data.arc = 359;
    hardpoint_data.arc_mid = 0;
    hardpoint_data.x_rel_tl = size.x; // put the manual gun front and center
    hardpoint_data.y_rel_tl = size.y / 2;

    auto weapon_e = spawn_weapon(r, e, hardpoint_data);
    r.emplace<ManualfireComponent>(weapon_e);
  }

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

    std::string hull_key = "Dinghy";
    auto transfer_scene_e = get_first<SelectSceneToSurviveScene>(r);
    if (transfer_scene_e != entt::null) {
      const auto& transfer_scene_c = r.get<SelectSceneToSurviveScene>(transfer_scene_e);
      hull_key = transfer_scene_c.chosen_boat;
    }

    // players
    const auto p1 = spawn_player(r, "actor_player", { 0, 0 }, 0, hull_key);
    // const auto p2 = spawn_player(r, "actor_player", { 16, 0 }, 1, hull_key);
    // const auto p3 = spawn_player(r, "actor_player", { 0, 16 }, 2, hull_key);
    // const auto p4 = spawn_player(r, "actor_player", { 16, 16 }, 3, hull_key);

    // inputs => players
    r.emplace<KeyboardComponent>(p1);
    r.emplace<SteamControllerComponent>(p1);
    // r.emplace<SteamControllerComponent>(p2);
    // r.emplace<SteamControllerComponent>(p3);
    // r.emplace<SteamControllerComponent>(p4);

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

  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("additive scene. scene set to: {}", scene_name).c_str());

  auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  scene.s = s; // done
};

} // namespace game2d