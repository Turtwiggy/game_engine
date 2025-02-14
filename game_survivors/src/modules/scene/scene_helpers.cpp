#include "scene_helpers.hpp"

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
#include "modules/combat/components.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/core_camera/components.hpp"
#include "modules/core_camera/orthographic.hpp"
#include "modules/core_colour/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"
#include "modules/effects_outline/outline_components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_autofire/autofire_components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_hulls/hulls_components.hpp"
#include "modules/system_hulls/hulls_helpers.hpp"
#include "modules/system_manualfire/manualfire_components.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_scene_splashscreen_move_to_menu/components.hpp"
#include "modules/system_screenshake/components.hpp"
#include "modules/system_spawner/spawner_components.hpp"
#include "modules/system_spawner/spawner_helpers.hpp"
#include "modules/system_spritestack/spritestack_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade_hp_regen/upgrade_hp_regen_components.hpp"
#include "modules/system_upgrade_xp_zone_size/upgrade_xp_zone_size_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_select/scene_select_components.hpp"
#include "modules/ui_scene_survive_level_up/ui_survive_level_up_components.hpp"
#include "modules/ui_scene_survive_timer/ui_survive_timer_components.hpp"

#include <magic_enum.hpp>

namespace game2d {

void
connect_parent_and_weapon(entt::registry& r, entt::entity e, entt::entity wep_e)
{
  auto& weapons_c = r.get_or_emplace<HasWeaponsComponent>(e);
  weapons_c.weapons.push_back(wep_e);
  r.emplace<HasParentComponent>(wep_e, HasParentComponent{ e });

  set_colour(r, wep_e, r.get<DefaultColour>(e).colour);
  set_position(r, wep_e, get_position(r, e));
};

entt::entity
spawn_weapon(entt::registry& r, const HardpointData& data)
{
  glm::vec2 weapon_size = { 3, 6 };

  const auto wep_e = spawn(r, "boat_default_weapon");
  give_life(r, wep_e, { 0, 0 }, weapon_size);
  r.emplace<TeamComponent>(wep_e, TeamComponent{ AvailableTeams::player });
  r.emplace<HardpointComponent>(wep_e, HardpointComponent{ data });

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
  r.emplace<BulletSize>(wep_e, BulletSize{ { 5, 2 } });
  r.emplace<BulletSpeed>(wep_e, 1.0f);
  r.emplace<BulletKnockback>(wep_e, 1);
  r.emplace<BulletBounce>(wep_e, 0); // no bounce by default

  set_z_index(r, wep_e, ZLayer::PLAYER_GUN_ABOVE_PLAYER);
  return wep_e;
};

bool
add_spritestack(entt::registry& r, entt::entity e, std::string sprite)
{
  std::vector<std::string> supported_spritestacks{
    "dinghy",
    "rhib",
    "pbr",
  };

  // i.e. which layer makes mose sense to have as the hitbox?
  std::vector<int> spritestack_base_layer{
    1,
    9,
    26,
  };

  auto it = std::find(supported_spritestacks.begin(), supported_spritestacks.end(), sprite);
  if (it == supported_spritestacks.end())
    return false; // oops! spritestack not implemented
  const auto idx = static_cast<int>(it - supported_spritestacks.begin());

  const auto& anims = get_first_component<SINGLE_Animations>(r);
  const auto [spritesheet, anim] = find_animation(anims, sprite + "_0"s);
  const int sprites_for_total_sprite = spritesheet.ny;

  entt::entity root_entity = entt::null;
  glm::vec2 pos{ 0, 0 };

  //
  // iterate from e.g. [-26, 12] for a ydepth of 38, where the center is 26 now
  //

  const int root_spritestack_img_idx = -spritestack_base_layer[idx];
  const int max = sprites_for_total_sprite + root_spritestack_img_idx;
  int counter = 0; // iterate through the spritestack frames

  for (int i = root_spritestack_img_idx; i < max; i++) {
    const auto i_as_str = std::to_string(counter++);
    const auto tag_str = sprite + "_"s + i_as_str;

    entt::entity spawned_e = entt::null;

    // this sets the SpriteComponent on the player
    if (i == 0)
      spawned_e = e;

    else {
      const auto sprite_e = create_transform(r, i_as_str);
      spawned_e = sprite_e;
    }

    // needs to be emplaced in order to maintain spritestack
    r.emplace<SpriteComponent>(spawned_e);

    set_sprite(r, spawned_e, sprite + "_"s + i_as_str);

    //
    // i goes from e.g. [-26, 12] on a 38 ydepth.
    // that works pretty well for z-index, where 0 represents "default"
    // however, z-index is sorted a.z_idx < b.z_idx,
    // but here i represents a sprite index, where -26 is the top sprite, not bottom.
    // hence, flip it, yo
    //
    // e.g. -26 should be 26
    // e.g. 0 should be 0
    // e.g. 12 should be -12
    //
    const int flipped_i = -1 * i;
    auto& t_c = r.get<TransformComponent>(spawned_e);
    t_c.z_index = flipped_i;

    SpritestackComponent spritestack_c(i);
    spritestack_c.spritestack_total = sprites_for_total_sprite;
    spritestack_c.root = e;
    spritestack_c.tag = tag_str;
    r.emplace<SpritestackComponent>(spawned_e, spritestack_c);
  }

  return true;
};

entt::entity
spawn_player(entt::registry& r, std::string key, glm::ivec2 pos, int num, std::string hull_key)
{
  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
  ShipHullData hull = get_hull(hulls_c, hull_key).value();
  const auto size = glm::vec2{ hull.width, hull.height };

  std::vector<entt::entity> weapons;

  // Spawn autofire weappons
  for (const auto& hardpoint_data : hull.hardpoints) {
    // HACK: overrode all arcs to 360 degrees. i.e. full coverage
    // hardpoint_data.arc = 360;
    // hardpoint_data.arc_mid = 0;
    auto weapon_e = spawn_weapon(r, hardpoint_data);
    r.emplace<AutofireComponent>(weapon_e);
    weapons.push_back(weapon_e);
  }

  // Spawn a manual weapon
  {
    HardpointComponent hardpoint_c;
    HardpointData hardpoint_data;
    hardpoint_data.key = "manual";
    hardpoint_data.arc = 360;
    hardpoint_data.arc_mid = 0;
    hardpoint_data.x_rel_tl = size.x / 2;
    hardpoint_data.y_rel_tl = size.y / 2;
    // hardpoint_data.x_rel_tl = size.x; // put the manual gun front and center
    // hardpoint_data.y_rel_tl = size.y / 2;
    auto weapon_e = spawn_weapon(r, hardpoint_data);
    r.emplace<ManualfireComponent>(weapon_e);
    weapons.push_back(weapon_e);
  }

  const auto e = spawn(r, key);
  give_life(r, e, pos, size);
  r.emplace<PlayerComponent>(e, num);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<MovementDirectComponent>(e);
  r.emplace<SetTransformRotationBasedOnPhysicsBody>(e);
  r.emplace<ActorSpeedComponent>(e, 0.01f);      // meters per second
  r.emplace<ActorHealthRegenComponent>(e, 0.0f); // hp per second

  // Add an xp zone with the config-defined size
  {
    auto fixture_def = get_fixture_def_by_tag(r, e, "fixture_xp_zone");
    ActorXpZoneSizeComponent xp_zone_c;
    xp_zone_c.radius_meters = pixels_to_meters(fixture_def.radius_in_pixels);
    r.emplace<ActorXpZoneSizeComponent>(e, xp_zone_c);
  }

  // r.emplace<SpriteOutline>(e);

  // add trail to the butt of the boat
  if (hull_key == "Dinghy") {
    const auto inset = 2.0f;
    {
      // const auto tl_offset = glm::vec2{ 0, size/2 };
      const auto tl_offset = glm::vec2{ inset, inset };
      const auto emitter_parent_e = create_transform(r, "trail-emitter-parent-l");
      r.emplace<DynamicTargetComponent>(emitter_parent_e, e);
      r.emplace<SetPositionAtDynamicTargetFromRotation>(emitter_parent_e, tl_offset);
      spawn_particle_emitter(r, "default_trail", emitter_parent_e);
    }
    {
      const auto tl_offset = glm::vec2{ inset, size.y - inset };
      const auto emitter_parent_e = create_transform(r, "trail-emitter-parent-r");
      r.emplace<DynamicTargetComponent>(emitter_parent_e, e);
      r.emplace<SetPositionAtDynamicTargetFromRotation>(emitter_parent_e, tl_offset);
      spawn_particle_emitter(r, "default_trail", emitter_parent_e);
    }
  }

  // Apply some drag, bro
  r.get<PhysicsBodyComponent>(e).body->SetLinearDamping(0.75f);

  // make a string lowercase?????
  std::string hull_lower = "";
  std::transform(
    hull_key.begin(), hull_key.end(), std::back_inserter(hull_lower), [](const auto& c) { return std::tolower(c); });

  // TODO: come up with something better to set sprites

  // If a spritestack is implemented, use that.
  r.remove<SpriteComponent>(e);
  if (add_spritestack(r, e, hull_lower))
    bool placeholder = true;
  else {
    SDL_Log("WARNING: not using spritestack model -- not impl");
    r.emplace<SpriteComponent>(e);
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
  };

  if (num == 0)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#cfc041")); // gold_yellow
  if (num == 1)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#da5bd6")); // magenta
  if (num == 2)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#00c420")); // green
  if (num == 3)
    r.emplace_or_replace<DefaultColour>(e, hex_to_srgb("#0096ff")); // blue
  set_colour(r, e, r.get<DefaultColour>(e).colour);

  auto player_fixture_e = get_fixture_by_tag(r, e, "fixture_player");
  r.emplace<PlayerFixtureComponent>(player_fixture_e);
  r.emplace<HealthComponent>(player_fixture_e, 10.0f, 10.0f);
  // r.emplace<DefenceComponent>(player_fixture_e, 0);

  // xp_zone fixture
  auto fixture_e = get_fixture_by_tag(r, e, "fixture_xp_zone");
  r.emplace<XpZoneComponent>(fixture_e);

  // upgrades...
  r.emplace<StatModifierComponent>(e);

  for (const auto& wep_e : weapons)
    connect_parent_and_weapon(r, e, wep_e);

  // This is more like which controller should control this player
  r.emplace<SteamControllerComponent>(e);

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
    // const auto units = load_units(r);
    // std::for_each(units.begin(), units.end(), [&r](const auto& u) { add_unit_to_entt(r, u); });

    // auto e = create_empty<TransformComponent>(r);
    // r.emplace<SpriteComponent>(e);
    // set_sprite(r, e, "STUDIO_TEXT_LOGO");
    // set_size(r, e, { 512, 256 });
    // set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::select) {
    create_empty<SINGLE_SelectSceneData>(r);
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
    // TODO: replace this player spawn system to a more dynamic
    // spawn system that lets players join halfway through
    const auto& controller_ui = get_first_component<SINGLE_SteamControllerGameState>(r);
    for (int i = 0; i < (int)controller_ui.handles.size(); i++) {
      auto handle = controller_ui.handles[i];
      if (handle == 0)
        continue;
      const auto p = spawn_player(r, "actor_player", { 0, 0 }, 0, hull_key);

      // assign handle
      r.get<SteamControllerComponent>(p).handle = handle;

      // Note: if the steamcontroller has a handle, controller overwrites keyboard
      if (i == 0)
        r.emplace<KeyboardComponent>(p);
    }

    // The survive timer that various spawners read from
    float seconds = 20 * 60;
    const auto survive_timer_e = create_empty<CooldownComponent>(r, CooldownComponent{ seconds, seconds });
    r.emplace<SurviveTimerComponent>(survive_timer_e);

    // const auto spawner_1_e = create_empty<CooldownComponent>(r);
    // r.emplace<EnemySpawnData>(spawner_1_e, exploder_data());

    const auto spawner_2_e = create_empty<CooldownComponent>(r);
    r.emplace<EnemySpawnData>(spawner_2_e, melee_enemy_1());

    // const auto spawner_3_e = create_empty<CooldownComponent>(r);
    // r.emplace<EnemySpawnData>(spawner_3_e, melee_enemy_2());

    // const auto spawner_4_e = create_empty<CooldownComponent>(r);
    // r.emplace<EnemySpawnData>(spawner_4_e, projectile_enemy());
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