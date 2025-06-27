#include "pch.hpp"

#include "scene_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actors/actor_hull/hull_components.hpp"
#include "modules/actors/actor_lighthouse/lighthouse_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/actors/actor_rock/rock_helpers.hpp"
#include "modules/actors/actor_rock/rock_system.hpp"
#include "modules/actors/actor_snake/snake_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_projectiles/projectile_components.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/camera/components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/colour/components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/lights/components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/events/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_ability/ability_components.hpp"
#include "modules/systems/system_audio_mix/audio_mix_components.hpp"
#include "modules/systems/system_autofire/autofire_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"
#include "modules/systems/system_particles/components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_helpers.hpp"
#include "modules/systems/system_player_out_of_bounds/player_out_of_bounds_components.hpp"
#include "modules/systems/system_scene_pressanykey_move_to_next/components.hpp"
#include "modules/systems/system_scene_splashscreen_move_to_next/components.hpp"
#include "modules/systems/system_screenshake/components.hpp"
#include "modules/systems/system_spawner/spawner_helpers.hpp"
#include "modules/systems/system_sprint/sprint_components.hpp"
#include "modules/systems/system_spritestack/spritestack_components.hpp"
#include "modules/systems/system_stats/stats_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_upgrade_dodge/upgrade_dodge_components.hpp"
#include "modules/systems/system_upgrade_hp_max/upgrade_hp_max_system.hpp"
#include "modules/systems/system_upgrade_hp_regen/upgrade_hp_regen_components.hpp"
#include "modules/systems/system_upgrade_xp_zone_size/upgrade_xp_zone_size_components.hpp"
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_components.hpp"
#include "modules/ui/ui_gameover/ui_gameover_components.hpp"
#include "modules/ui/ui_scene_main_menu/helpers.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_components.hpp"
#include "modules/ui/ui_scene_select/scene_select_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_helpers.hpp"
#include "modules/ui/ui_scene_survive_timer/ui_survive_timer_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_worldspace_text/helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

void
connect_parent_and_weapon(entt::registry& r, entt::entity e, entt::entity wep_e)
{
  auto& weapons_c = r.get_or_emplace<HasChildrenComponent>(e);
  weapons_c.children.push_back(wep_e);
  r.emplace<HasParentComponent>(wep_e, HasParentComponent{ e });

  // set_colour(r, wep_e, r.get<DefaultColour>(e).colour);
  set_position(r, wep_e, get_position(r, e));
};

bool
add_spritestack(entt::registry& r, entt::entity e, std::string sprite)
{
  const std::vector<std::string> supported_spritestacks{
    "dinghy",
    "rhib",
    "pbr",
  };

  // i.e. which layer makes mose sense to have as the hitbox?
  // note: ignoring {0, 0}. so if dinghy_1 is frame {0, 1} = 0,
  // the int value 1 in this vector represents 1 frame after that.
  const std::vector<int> spritestack_base_layer{
    1,
    9,
    26,
  };

  auto it = std::find(supported_spritestacks.begin(), supported_spritestacks.end(), sprite);
  if (it == supported_spritestacks.end())
    return false; // oops! spritestack not implemented
  const auto idx = static_cast<int>(it - supported_spritestacks.begin());

  const auto& anims = SINGLE_Animations::instance;
  const auto [spritesheet, anim] = find_animation(anims, sprite + "_1"s);
  const int sprites_for_total_sprite = spritesheet.ny - 1; // note: -1 because {0, 0} should be empty

  entt::entity root_entity = entt::null;
  glm::vec2 pos{ 0, 0 };

  // iterate from e.g. [-26, 12] for a ydepth of 38, where the center is 26 now
  const int root_spritestack_img_idx = -spritestack_base_layer[idx];
  const int max = sprites_for_total_sprite + root_spritestack_img_idx;

  // iterate through the spritestack frames
  // note: {0, 0} is an empty frame, so start the counter at 1.
  int counter = 1;

  for (int i = root_spritestack_img_idx; i < max; i++) {
    const auto i_as_str = std::to_string(counter++);
    const auto tag_str = sprite + "_"s + i_as_str;

    entt::entity spawned_e = entt::null;

    // this sets the SpriteComponent on the player
    if (i == 0)
      spawned_e = e;

    else
      spawned_e = create_transform(r, tag_str);

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
spawn_player(entt::registry& r, std::string key, int num, std::string hull_key, std::string weapon_key)
{
  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
  const auto& weps_c = get_first_component<SINGLE_Weapons>(r);
  const auto pos = rnd_position_in_map_but_not_inside_players_or_islands(r);

  auto get_key = []<typename T>(const std::vector<T>& data, const std::string& key) -> std::optional<T> {
    const auto it = std::find_if(data.begin(), data.end(), [&key](const T& item) { return item.key == key; });
    if (it == data.end())
      return std::nullopt;
    return (*it);
  };
  const auto hull = get_key(hulls_c.hulls, hull_key).value();
  const auto weapon_data = get_key(weps_c.weapons, weapon_key).value();
  const auto hull_size = glm::vec2{ hull.width, hull.height };

  std::vector<entt::entity> weapons;

  const auto e = spawn(r, key);
  r.emplace<StatModifierComponent>(e); //  upgrades

  // Spawn autofire weappons
  for (const auto& hardpoint_data : hull.hardpoints) {
    // HACK: overrode all arcs to 360 degrees. i.e. full coverage
    // hardpoint_data.arc = 360;
    // hardpoint_data.arc_mid = 0;
    auto wep_e = spawn_weapon(r, e, weapon_data, weapon_key);
    r.emplace<HardpointComponent>(wep_e, HardpointComponent{ hardpoint_data });
    r.emplace<WeaponDef>(wep_e, get_weapon_def(r, e, wep_e));
    r.emplace<BulletDef>(wep_e, get_bullet_def(r, e, wep_e));

    if (weapon_data.type_as_enum == WEAPON_TYPE::PROJECTILE)
      r.emplace<AutofireComponent>(wep_e);
    if (weapon_data.type_as_enum == WEAPON_TYPE::DEPLOY)
      r.emplace<WeaponSeaTurret>(wep_e);

    weapons.push_back(wep_e);

    if (weapon_data.type_as_enum == WEAPON_TYPE::DEPLOY)
      break; // only spawn 1 deployer

    // break; // spawn boats with only 1 gun
  }

  // Spawn a manual weapon
  // {
  //   HardpointComponent hardpoint_c;
  //   HardpointData hardpoint_data;
  //   hardpoint_data.key = "manual";
  //   hardpoint_data.arc = 360;
  //   hardpoint_data.arc_mid = 0;
  //   hardpoint_data.x_rel_tl = size.x / 2;
  //   hardpoint_data.y_rel_tl = size.y / 2;
  //   // hardpoint_data.x_rel_tl = size.x; // put the manual gun front and center
  //   // hardpoint_data.y_rel_tl = size.y / 2;
  //   auto weapon_e = spawn_weapon(r, hardpoint_data);
  //   r.emplace<ManualfireComponent>(weapon_e);
  //   weapons.push_back(weapon_e);
  // }

  give_life(r, e, pos, hull_size);
  r.emplace<PlayerComponent>(e, num);
  r.emplace<CameraFollow>(e);
  r.emplace<TeamComponent>(e, TeamComponent{ AvailableTeams::player });
  r.emplace<MovementDirectComponent>(e);
  r.emplace<RotateToVelocityComponent>(e);
  r.emplace<SetTransformRotationBasedOnPhysicsBody>(e);
  r.emplace<OutOfBoundsTimer>(e);
  r.emplace<AbilityComponent>(e);
  r.emplace<HullKeyComponent>(e, hull_key);
  r.emplace<LightEmitterComponent>(e);
  r.emplace<LightTypeCircle>(e);

  // Upgradeable stats
  r.emplace<ActorHealthRegenComponent>(e, 0.0f); // hp per second
  r.emplace<ActorDodgeComponent>(e, 0.0f);       // dodge percent
  r.emplace<ActorStaminaComponent>(e);

  // Add an xp zone with the config-defined size
  {
    auto fixture_def = get_fixture_def_by_tag(r, e, "fixture_xp_zone");

    ActorXpZoneSizeComponent xp_zone_c;
    // xp_zone_c.radius_meters = pixels_to_meters(fixture_def.size[0].x);
    xp_zone_c.radius_meters = pixels_to_meters(hull_size.x * 4.0f);

    // update_circle_fixture_size(r, e, f);
    r.emplace<ActorXpZoneSizeComponent>(e, xp_zone_c);
  }

  // r.emplace<SpriteOutline>(e);

  const auto add_trail = [&r, e](glm::vec2 offset) {
    const auto tl_offset = offset;
    const auto emitter_parent_e = create_transform(r, "trail-emitter-parent-l");
    r.emplace<DynamicTargetComponent>(emitter_parent_e, e);
    r.emplace<SetPositionAtDynamicTargetFromRotation>(emitter_parent_e, tl_offset);

    RequestToSpawnParticles req;
    req.key = "default_trail";
    req.parent = emitter_parent_e;
    spawn_particle_emitter(r, req);
  };

  // add trail to the butt of the boat
  //
  if (hull_key == "dinghy") {
    const auto inset = 2.0f;
    add_trail({ inset, inset });
    add_trail({ inset, hull_size.y - inset });
  }
  if (hull_key == "pbr") {
    const auto inset = 2.0f;
    add_trail({ inset, inset });
    add_trail({ inset, hull_size.y - inset });
  }
  if (hull_key == "rhib") {
    const auto inset = 3.0f;
    add_trail({ inset, inset });
    add_trail({ inset, hull_size.y - inset });
  }
  if (hull_key == "trimaran") {
    const auto inset = 2.0f;
    add_trail({ inset, inset });
    add_trail({ inset, hull_size.y * 0.5f });
    add_trail({ inset, hull_size.y - inset });
  }

  // Apply some drag, bro
  b2Body_SetLinearDamping(r.get<PhysicsBodyComponent>(e).bodyId, 0.75f);

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
    if (hull_key == "dinghy")
      set_sprite(r, e, "hull_dinghy");
    if (hull_key == "rhib")
      set_sprite(r, e, "hull_rhib");
    // if (hull_key == "constitution")
    //   set_sprite(r, e, "hull_constitution");
    if (hull_key == "pbr")
      set_sprite(r, e, "hull_pbr");
    if (hull_key == "trimaran")
      set_sprite(r, e, "hull_trimaran");
  };

  // convert boat to hp?
  float hp = 10.0f;
  // take hp as a percent of the boat size?
  hp = (hull_size.x * hull_size.y) / 100.0f;
  SDL_Log("Boat HP: %f", hp);

  // set the player colour.
  const auto col = default_player_colours[num];
  set_colour(r, e, r.get<DefaultColour>(e).colour);

  auto player_fixture_e = get_fixture_by_tag(r, e, "fixture_player");
  r.emplace<PlayerFixtureComponent>(player_fixture_e);
  r.emplace<HealthComponent>(player_fixture_e, hp, hp);
  // r.emplace<DefenceComponent>(player_fixture_e, 0);

  // xp_zone fixture
  auto fixture_e = get_fixture_by_tag(r, e, "fixture_xp_zone");
  r.emplace<XpZoneComponent>(fixture_e);

  for (const auto wep_e : weapons)
    connect_parent_and_weapon(r, e, wep_e);

  // This is more like which controller should control this player
  r.emplace<SteamControllerComponent>(e);

  return e;
};

void
spawn_players(entt::registry& r)
{
  std::vector<HullChoice> hull_keys = {
    HullChoice{ .player_idx = 0, .player_boat_key = "dinghy" },
    HullChoice{ .player_idx = 1, .player_boat_key = "dinghy" },
    HullChoice{ .player_idx = 2, .player_boat_key = "dinghy" },
    HullChoice{ .player_idx = 3, .player_boat_key = "dinghy" },
  };

  const auto transfer_scene_e = get_first<SelectSceneToSurviveScene>(r);
  if (transfer_scene_e != entt::null) {
    const auto& transfer_scene_c = r.get<SelectSceneToSurviveScene>(transfer_scene_e);
    hull_keys.clear();
    hull_keys = transfer_scene_c.chosen_boats;
    r.destroy(transfer_scene_e);
  }

  // players
  // TODO: replace this player spawn system to a more dynamic
  // spawn system that lets players join halfway through
  const auto& controller_ui = get_first_component<SINGLE_SteamControllerGameState>(r);
  const int n_max_players = 4;

  for (int i = 0; i < n_max_players; i++) {

    // controller connected for players
    const auto handle = controller_ui.handles[i];
    const bool handle_joined = handle_is_joined(controller_ui, handle);

    // note: always has 1 player using keyboard
    if (i > 0 && !handle_joined)
      continue; // no controller for p2-4

    // validate weapons are set
    const auto boat_str = hull_keys[i].player_boat_key;
    if (boat_str == "")
      throw std::runtime_error("boat_str not set");
    const auto weapon_str = hull_keys[i].player_gun_key;
    if (weapon_str == "")
      throw std::runtime_error("weapon_str not set");
    SDL_Log("player wants to spawn with %s %s", boat_str.c_str(), weapon_str.c_str());

    const auto p = spawn_player(r, "actor_player", i, boat_str, weapon_str);

    if (handle_joined)
      r.get<SteamControllerComponent>(p).handles.push_back(handle);

    if (i == 0)
      r.emplace<KeyboardComponent>(p);
  }

  load_persistent_upgrades_and_apply_to_player(r);

  // bugfix: this makes sure the player has the correct health on spawn after loading upgrades.
  update_upgrade_hp_max_system(r);
  const auto player_view = r.view<PlayerFixtureComponent, HealthComponent>();
  for (const auto& [e, player_fixture_c, hp_c] : player_view.each())
    hp_c.hp = hp_c.max_hp;
}

void
spawn_lighthouses(entt::registry& r)
{
  // create a "lighthouse" on each of the islands.
  for (const auto [e, island_c, bb_c] : r.view<const RockComponent, const BoundingBoxComponent>().each()) {
    const auto center = 0.5f * (bb_c.br + bb_c.tl);
    auto lighthouse_e = spawn(r, "actor_lighthouse");
    give_life(r, lighthouse_e, center, { 32, 32 });
    set_sprite(r, lighthouse_e, "ARROW_RIGHT");
    // add_spritestack(r, lighthouse_e, "lighthouse"); // todo
    r.emplace<LighthouseComponent>(lighthouse_e);
    r.emplace<LightEmitterComponent>(lighthouse_e);
    r.emplace<LightTypeWedge>(lighthouse_e);
    auto popup_e = create_popup(r, center, "Lighthouse");
    r.remove<EntityTimedLifecycle>(popup_e);
    r.get<WiggleUpAndDown>(popup_e).amplitude = 1.0f;
  }
};

void
spawn_islands(entt::registry& r)
{

  auto& data_c = get_first_component<SINGLE_ModifiersData>(r);
  auto rock_opt = get_modifier_option(r, MODIFIER_OPTIONS::ROCKS);
  if (dynamic_cast<Option_Rocks*>(rock_opt.get())->populate_rocks) {
    create_empty<RequestGenerateRocks>(r);
    // need islands and rocks to exist before player spawns,
    // to determine player spawn location
    update_actor_rocks_system(r);
  }
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
  SINGLE_CurrentScene::instance = {};
  SINGLE_ScreenshakeComponent::instance = {};
  create_empty<SINGLE_EntityBinComponent>(r);
  create_empty<SINGLE_GameStateComponent>(r);
  create_empty<SINGLE_InputComponent>(r);
  create_empty<SINGLE_ImSprite>(r);
  begin_frame_sprite(r); // initialize cached for sprites

  // The first and only transform should be the camera
  const auto camera_e = get_first<OrthographicCamera>(r);
  r.get<TransformComponent>(camera_e).position = { 0, 0, 0 };
  r.get<TransformComponent>(camera_e).scale = { 0, 0, 0 };

  audio::sdl_mixer::stop_all_audio(r);

  // clear all the inputs when moving scene
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  auto& g_input_c = r.get<InputComponent>(g_input_e);
  {
    g_input_c.pause.clear();
    g_input_c.ability1.clear();
    g_input_c.ability2.clear();
    g_input_c.dpad_u.clear();
    g_input_c.dpad_d.clear();
    g_input_c.dpad_l.clear();
    g_input_c.dpad_r.clear();
    g_input_c.button_n.clear();
    g_input_c.button_s.clear();
    g_input_c.button_e.clear();
    g_input_c.button_w.clear();
  }

  if (s == Scene::splashscreen) {
    create_empty<SINGLE_SplashScreen>(r);

    auto e = create_empty<TransformComponent>(r);
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, "STUDIO_LOGO");
    set_size(r, e, { 512, 512 });
    set_position(r, e, { 0, 0 }); // center
  }

  if (s == Scene::pressanykey) {
    create_empty<SINGLE_PressAnykeyScene>(r);
  }

  if (s == Scene::menu) {
    const int num_players = 4;

    create_empty<SINGLE_MainMenuUI>(r);
    create_empty<SINGLE_PersistentUpgradesMenuUI>(r);
    SINGLE_MainMenuAnimatedData menu_anim_c;
    menu_anim_c.data.resize(num_players);
    create_empty<SINGLE_MainMenuAnimatedData>(r, menu_anim_c);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "MENU_0", .looping = true });
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "WATER_AMBIENCE_0", .looping = true });
    init_oh_buoy_header_text(r);

    // load player's saved units
    // const auto units = load_units(r);
    // std::for_each(units.begin(), units.end(), [&r](const auto& u) { add_unit_to_entt(r, u); });

    // create a fluidsim sprite.
    // auto& ri_c = SINGLE_RendererInfo::instance;
    // const auto fluidsim_e = r.create();
    // r.emplace<TagComponent>(fluidsim_e, "fluidsim_e");
    // r.emplace<ItemKey>(fluidsim_e, "empty");
    // give_life(r, fluidsim_e, { 0, 0 }, { 4096, 4096 });
    // auto& sprite_c = r.get<SpriteComponent>(fluidsim_e);
    // sprite_c.tex_unit = ri_c.passes[(int)get_pass_idx(ri_c, PassName::fluid_sim)].texs[0].tex_unit.unit;
    // sprite_c.colour = engine::LinearColour{ 0.0f, 0.0f, 0.0f, 1.0f };
  }

  if (s == Scene::select_modifiers) {
    init_oh_buoy_header_text(r);
    create_empty<RequestToShowModifierMenu>(r);

    auto& data_c = get_first_component<SINGLE_ModifiersData>(r);
  }

  if (s == Scene::select_ships) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "SELECT_0", .looping = true });
    init_oh_buoy_header_text(r);
    create_empty<SINGLE_SelectSceneData>(r);
  }

  if (s == Scene::survive) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "WATER_AMBIENCE_0", .looping = true });
    create_empty<SINGLE_SurviveStatsComponent>(r);
    create_empty<Effect_GridComponent>(r);
    create_empty<SINGLE_XpComponent>(r);
    create_empty<SINGLE_LevelUpUI>(r);
    create_empty<SINGLE_GameoverUI>(r);
    create_empty<RequestGameTrack>(r);

    // Reset temporary gold
    auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);
    gold_c.temp_amount = 0;

    // The survive timer that various spawners read from
    const auto survive_timer_e = create_empty<SurviveTimerComponent>(r);

    spawn_islands(r); // before spawn_players
    spawn_players(r);
    spawn_lighthouses(r);

    // populate spawners from configs
    create_empty<SpawnerLiveData>(r);
    init_spawners(r);
  }

  if (s == Scene::develop_snake) {
    create_empty<CameraFreeMove>(r);

    const auto p = spawn_player(r, "actor_player", 0, "dinghy", "weapon_deck_cannon");

    const auto& controller_ui = get_first_component<SINGLE_SteamControllerGameState>(r);
    for (int i = 0; i < (int)controller_ui.handles.size(); i++) {
      auto handle = controller_ui.handles[i];
      r.get<SteamControllerComponent>(p).handles.push_back(handle);
      break;
    }

    create_snake(r); // create a snake yo
  }

  if (s == Scene::develop_islands) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "WATER_AMBIENCE_0", .looping = true });
    create_empty<SINGLE_SurviveStatsComponent>(r);
    create_empty<Effect_GridComponent>(r);
    create_empty<SINGLE_XpComponent>(r);
    create_empty<SINGLE_LevelUpUI>(r);
    create_empty<SINGLE_GameoverUI>(r);
    create_empty<RequestGameTrack>(r);

    spawn_islands(r); // before spawn_players

    // spawn_players(r);
    const auto p = spawn_player(r, "actor_player", 0, "dinghy", "weapon_deck_cannon");
    r.emplace<KeyboardComponent>(p);
    const auto& controller_ui = get_first_component<SINGLE_SteamControllerGameState>(r);
    for (int i = 0; i < (int)controller_ui.handles.size(); i++) {
      auto handle = controller_ui.handles[i];
      r.get<SteamControllerComponent>(p).handles.push_back(handle);
      break;
    }

    generate_rocks_interior(r);
    spawn_lighthouses(r);
  }

  auto& scene = SINGLE_CurrentScene::instance;
  scene.s = s; // done
};

void
move_to_scene_additive(entt::registry& r, const Scene& s)
{
  audio::sdl_mixer::stop_all_audio(r);

  const auto scene_name = std::string(magic_enum::enum_name(s));
  SDL_Log("%s", std::format("additive scene. scene set to: {}", scene_name).c_str());

  auto& scene = SINGLE_CurrentScene::instance;
  scene.s = s; // done
};

} // namespace game2d