#include "pch.hpp"

#include "game.hpp"

#include "engine/audio/audio_system.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/system.hpp"
#include "engine/imgui/ui_imgui_colours.hpp"
#include "engine/lifecycle/lifecycle_system.hpp"
#include "engine/physics/physics_system.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actors/actor_enemy_charger/enemy_charger_system.hpp"
#include "modules/actors/actor_enemy_grower/enemy_grower_system.hpp"
#include "modules/actors/actor_player/actor_player_system.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_system.hpp"
#include "modules/actors/actor_snake/snake_helpers.hpp"
#include "modules/actors/actor_snake_projectiles/actor_snake_projectiles_system.hpp"
#include "modules/actors/actor_swarmlord/enemy_swarmlord_system.hpp"
#include "modules/actors/actor_weapon/weapon_helpers.hpp"
#include "modules/combat/combat_elemental_damage/elemental_damage_system.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_system.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_system.hpp"
#include "modules/core/animations/rotate_system.hpp"
#include "modules/core/animations/wiggle/wiggle_up_and_down.hpp"
#include "modules/core/animator/animator_system.hpp"
#include "modules/core/camera/camera_system.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_components.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/system.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
#include "modules/effect_crt/crt_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/events/events_core/events_system.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/steam/steam_helpers.hpp"
#include "modules/steam_debug_ui/steam_debug_ui_system.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/systems/system_ability/ability_system.hpp"
#include "modules/systems/system_alpha_based_on_lifecycle/alpha_based_on_lifecycle_system.hpp"
#include "modules/systems/system_autofire/autofire_system.hpp"
#include "modules/systems/system_cooldown/cooldown_system.hpp"
#include "modules/systems/system_create_item/create_item_system.hpp"
#include "modules/systems/system_death_throes/death_throes_system.hpp"
#include "modules/systems/system_enemy_projectile/enemy_projectile_system.hpp"
#include "modules/systems/system_gameover/gameover_system.hpp"
#include "modules/systems/system_hardpoint_arcs/hardpoint_arcs_system.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_helpers.hpp"
#include "modules/systems/system_input_open_ui/input_open_ui_system.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_item_gold/gold_helpers.hpp"
#include "modules/systems/system_move_to_target_via_lerp/move_to_target_via_lerp_system.hpp"
#include "modules/systems/system_particles/particle_system.hpp"
#include "modules/systems/system_particles_on_death/system.hpp"
#include "modules/systems/system_pause/pause_helpers.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_helpers.hpp"
#include "modules/systems/system_physics_apply_force/physics_apply_force_system.hpp"
#include "modules/systems/system_player_out_of_bounds/player_out_of_bounds_system.hpp"
#include "modules/systems/system_quit/quit_system.hpp"
#include "modules/systems/system_scene_pressanykey_move_to_next/scene_pressanykey_move_to_next_system.hpp"
#include "modules/systems/system_scene_splashscreen_move_to_next/system.hpp"
#include "modules/systems/system_screenshake/system.hpp"
#include "modules/systems/system_spawner/spawner_components.hpp"
#include "modules/systems/system_spawner/spawner_helpers.hpp"
#include "modules/systems/system_spawner/spawner_system.hpp"
#include "modules/systems/system_spritestack/spritestack_system.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_upgrade_hp_max/upgrade_hp_max_system.hpp"
#include "modules/systems/system_upgrade_hp_regen/upgrade_hp_regen_system.hpp"
#include "modules/systems/system_upgrade_xp_zone_size/upgrade_xp_zone_size_system.hpp"
#include "modules/systems/system_weapon_sea_turret/weapon_sea_turret_system.hpp"
#include "modules/ui/ui_ability_system/ui_ability_system.hpp"
#include "modules/ui/ui_audio/system.hpp"
#include "modules/ui/ui_back_button/ui_back_button_system.hpp"
#include "modules/ui/ui_blur/ui_blur_system.hpp"
#include "modules/ui/ui_collisions/system.hpp"
#include "modules/ui/ui_colours/ui_colours_system.hpp"
#include "modules/ui/ui_debug_effects/ui_debug_effects_system.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_system.hpp"
#include "modules/ui/ui_debug_spawner/ui_debug_spawner_system.hpp"
#include "modules/ui/ui_debug_upgrades/ui_debug_upgrades_system.hpp"
#include "modules/ui/ui_debug_weapons/ui_debug_weapons_system.hpp"
#include "modules/ui/ui_fps_counter/system.hpp"
#include "modules/ui/ui_gameover/ui_gameover_system.hpp"
#include "modules/ui/ui_hierarchy/system.hpp"
#include "modules/ui/ui_popup_controller_disconnected/ui_popup_controller_disconnected_components.hpp"
#include "modules/ui/ui_popup_controller_disconnected/ui_popup_controller_disconnected_system.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_system.hpp"
#include "modules/ui/ui_popup_pause/ui_popup_pause_components.hpp"
#include "modules/ui/ui_popup_pause/ui_popup_pause_system.hpp"
#include "modules/ui/ui_raws/ui_raws_system.hpp"
#include "modules/ui/ui_scene_header/ui_scene_header_system.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_system.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_system.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_system.hpp"
#include "modules/ui/ui_scene_select/scene_select_system.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_components.hpp"
#include "modules/ui/ui_scene_select_modifiers/select_modifiers_system.hpp"
#include "modules/ui/ui_scene_survive/scene_survive_system.hpp"
#include "modules/ui/ui_scene_survive_hp_bars/ui_survive_hp_bars.hpp"
#include "modules/ui/ui_scene_survive_info/ui_survive_info_system.hpp"
#include "modules/ui/ui_scene_survive_timer/ui_survive_timer_system.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_components.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_system.hpp"
#include "modules/ui/ui_scene_survive_xp_bar/ui_survive_xp_bar_system.hpp"
#include "modules/ui/ui_worldspace_text/system.hpp"
#include "resources/resources.hpp"

#if defined(_DEBUG)
#include <tracy/Tracy.hpp>
#endif

namespace game2d {
using namespace std::literals;

bool custom_mouse_cursor = false;

void
init(engine::SINGLE_Application& app, entt::registry& r)
{
  init_events_system(r);
  init_fonts_system();

  // hide default cursor
  if (custom_mouse_cursor) {
    // io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    // auto result = SDL_ShowCursor(SDL_DISABLE);
    // if (result < 0)
    //   SDL_Log("Failed to hide system cursor: %s", SDL_GetError());
  }

  // Init steam before loading textures, because
  // some of the button icon glyph / textures are loaded via steam.
  init_input_system(r);

#if defined(USE_STEAM)
  init_steam(r);
  init_steam_input(r);
  create_persistent<SteamOverlayManager>(r);
#endif

  {
    SINGLE_RendererInfo ri = get_default_textures();
    create_persistent<SINGLE_RendererInfo>(r, ri);
    create_persistent<OrthographicCamera>(r);
    r.emplace<TransformComponent>(get_first<OrthographicCamera>(r));
    init_render_system(app, r); // load textures
  }
  {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    SINGLE_Animations anims;
    for (const auto& tex : ri.user_textures)
      load_sprites(anims, tex);
    create_persistent<SINGLE_Animations>(r, anims);
  }

  create_persistent<SINGLE_PauseMenuState>(r);
  create_persistent<SINGLE_DebugMenuBar>(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  create_persistent<SINGLE_Hulls>(r, load_hulls("assets/raws/hulls/"));
  create_persistent<SINGLE_PersistentUpgrades>(r, load_upgrades("assets/raws/persistent_upgrades.jsonc"));
  create_persistent<SINGLE_Weapons>(r, load_weapons("assets/raws/weapons.jsonc"));
  create_persistent<SINGLE_OnDiskSpawners>(r, load_spawns("assets/raws/spawns.jsonc"));
  create_persistent<SINGLE_UpgradeToName>(r, load_upgrade_names("assets/raws/upgrade_names.jsonc"));

  create_persistent<SINGLE_EffectCrt>(r);
  create_persistent<SINGLE_FixedUpdateInputHistory>(r);
  create_persistent<SINGLE_SteamControllerGameState>(r);
  create_persistent<SINGLE_PostFixedUpdateCallbacks>(r);

  create_persistent<SINGLE_ModifiersData>(r);
  create_persistent<SINGLE_DisconnectedControllerUI>(r);
  create_persistent<SINGLE_UIScaling>(r); // HMM: could make a setting
  create_persistent<SINGLE_GameOptions>(r);
  create_persistent<SINGLE_OnDiskData>(r, savefile_load_disk(r));
  create_persistent<SINGLE_GoldComponent>(r, load_gold_from_disk(r)); // easy to cheat! have fun.

  // the "global" input component, which processes all inputs from keyboard & controllers
  // note: players also have an InputComponent attached
  const auto input_e = create_persistent<InputComponent>(r);
  r.emplace<KeyboardComponent>(input_e);
  r.emplace<SteamControllerComponent>(input_e);

  move_to_scene_start(r, Scene::splashscreen);
};

void
init_slow(engine::SINGLE_Application& app, entt::registry& r)
{
  create_persistent<SINGLE_AudioComponent>(r, get_default_audio());

  init_audio_system(r);
};

void
duplicate_held_input(SINGLE_FixedUpdateInputHistory& fixed_input)
{
  // get last tick held inputs
  auto& last_tick_inputs = fixed_input.history[fixed_input.fixed_tick - 1];

  // append them to this tick
  auto& i = fixed_input.history[fixed_input.fixed_tick];

  // only append the held states
  const auto is_held = [](const InputEvent& e) { return e.state == InputState::held; };
  const auto [first, last] = std::ranges::remove_if(last_tick_inputs, is_held);
  i.insert(i.end(), first, last);
};

void
fixed_update(engine::SINGLE_Application& app, entt::registry& r, const uint64_t milliseconds_dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& input = get_first_component<SINGLE_InputComponent>(r);
  auto& fixed_input = get_first_component<SINGLE_FixedUpdateInputHistory>(r);

  // move inputs from Update() to this FixedUpdate() tick
  fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);

  // If there's no new Update since the last FixedUpdate(),
  // held state wont be generated for the fixed tick.
  // Duplicate the held inputs for the last frame.
  if (!input.update_since_last_fixed_update)
    duplicate_held_input(fixed_input);
  input.update_since_last_fixed_update = false;

  // move unprocessed inputs from Update() to this FixedUpdate() tick
  const auto inputs = std::move(fixed_input.history[fixed_input.fixed_tick]);
  fixed_input.history.clear();
  fixed_input.history[fixed_input.fixed_tick] = std::move(inputs);

  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (state.state == GameState::PAUSED)
    return; // note: this ignores inputs

  if (require_pause(r))
    return;

  const auto mouse_pos = mouse_position_in_worldspace(r);

  // destroy/create objects
  update_lifecycle_system(r, milliseconds_dt);
  update_physics_apply_force_system(r);
  update_physics_system(r, milliseconds_dt);
  update_events_system(r); // dispatch events
  update_create_item_system(r);
  fixed_update_player_controller_system(r, milliseconds_dt, mouse_pos);

  // fixed_input.fixed_tick += 1;
};

void
update(engine::SINGLE_Application& app, entt::registry& r, const uint64_t milliseconds_dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  const float dt = milliseconds_dt / 1000.0f;
  const auto mouse_pos = mouse_position_in_worldspace(r);

  begin_frame_sprite(r);
  update_sdl_event_system(app, r); // sets update_since_last_fixed_update

#if defined(USE_STEAM)
  SteamAPI_RunCallbacks();
  update_steam_input(r);
#endif

  update_camera_system(r, dt);
  update_audio_system(r, dt);
  update_player_controller_system(r, mouse_pos);
  update_screenshake_system(r, dt);
  update_input_open_ui_system(r);
  update_events_system(r); // dispatch events
  update_quit_system(r, app);

  if (scene.s == Scene::pressanykey)
    update_scene_pressanykey_move_to_next_system(r, dt);

  if (scene.s == Scene::splashscreen)
    update_scene_splashscreen_move_to_next_system(app, r, dt);

  // pause due to gamelogic
  bool pause = require_pause(r);

  update_hardpoint_arcs_system(r);
  update_sprite_spritestack_system(r, dt);

  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (state.state != GameState::PAUSED && !pause) {
    update_actor_rocks_system(r); // before update_spawner_system
    update_animator_system(r, dt);
    update_animation_rotate_system(r, dt);
    // update_manualfire_system(r, dt);
    update_combat_elemental_damage_system(r, dt);
    update_combat_scale_on_hit_system(r, dt);
    update_cooldown_system(r, milliseconds_dt);
    update_gun_follow_player_system(r, mouse_pos, dt);
    update_move_to_target_via_lerp(r, dt);
    update_particle_system(r, dt);
    update_spawn_particles_on_death_system(r);
    update_wiggle_up_and_down_system(r, dt);
    update_spawner_system(r, dt);
    update_alpha_based_on_lifecycle_system(r);
    // update_sprint_system(r, dt);
    update_gameover_system(r);
    update_player_out_of_bounds_system(r, dt);

    update_autofire_system(r, dt); // prefer after hardpoints_system
    update_weapon_sea_turret_system(r, dt);
    update_ability_system(r, dt);

    update_death_throes_system(r, dt);
    update_enemy_charger_system(r);
    update_enemy_grower_system(r, dt);
    update_enemy_projectile_system(r);
    update_enemy_swarmlord_system(r);
    update_snake(r, mouse_pos, dt);
    update_actor_snake_projectiles_system(r);

    update_upgrade_hp_max_system(r);
    update_upgrade_hp_regen_system(r, dt);
    update_upgrade_xp_zone_size_system(r);
  }

  // update ui scaling
  {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const float base_x = 1280; // note: this was the res the ui was created at
    const float base_y = 720;
    const float scale = ri.viewport_size_render_at.y / base_y;
    auto& ui_scale = get_first_component<SINGLE_UIScaling>(r);
    ui_scale.scaling = scale <= 1.0 ? 1.0f : 1.25f;

#if defined(_DEBUG)
    // auto& ui_scale = get_first_component<SINGLE_UIScaling>(r);
    // ui_scale.scaling = 1.0f;
    // imgui_draw_float("ui_scale", ui_scale.scaling);
#endif
  }

  update_ui_fps_counter_system(r);
  update_ui_popup_pause_system(app, r);
  update_ui_popup_options_system(app, r);
  update_ui_popup_controller_disconnected_system(r);
  update_ui_worldspace_text_system(r);
  update_ui_back_button_system(r);
  update_ui_scene_header_system(r);

  // if (scene.s == Scene::pressanykey)
  //   update_ui_scene_press_any_key(r);

  if (scene.s == Scene::menu) {
    update_ui_scene_main_menu(app, r);
    update_ui_scene_main_menu_controllerinfo_system(r, dt);
    update_ui_scene_upgrades_system(r);
  }

  if (scene.s == Scene::select_modifiers)
    update_ui_scene_select_modifiers_system(r);

  if (scene.s == Scene::select_ships)
    update_ui_scene_select_system(r, dt);

  if (scene.s == Scene::survive) {
    update_ui_scene_survive_system(r);
    update_ui_survive_timer_system(r);
    update_ui_survive_info_system(r);
    update_ui_survive_hp_bars_system(r);
    update_ui_survive_xp_bar_system(r);
    update_ui_survive_upgrade_system(r);
    update_ui_gameover_system(r);
    update_ui_ability_system(r);
  }

#if defined(_DEBUG)
  static bool show_settings_ui = true;
#else
  static bool show_settings_ui = false;
#endif
  if (show_settings_ui) {
    update_ui_debug_menubar_system(r);
    update_ui_debug_upgrades_system(r);
    update_ui_imgui_colours_system(r);
    update_physics_apply_force_debug_ui(r);

    auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);

    auto demo_state = gesert_menubar_state(menu_c, "ImGui Demo");
    if (demo_state.enabled)
      ImGui::ShowDemoWindow(NULL);

    auto audio_state = gesert_menubar_state(menu_c, "Audio");
    if (audio_state.enabled)
      update_ui_audio_system(r);

#if defined(USE_STEAM)
    auto ui_steam_state = gesert_menubar_state(menu_c, "Steam");
    if (ui_steam_state.enabled)
      update_steam_debug_ui_system(r);
#endif

    update_ui_colours_system(r);
    update_ui_debug_spawner_system(r);
    update_ui_debug_weapons_system(r);
    update_ui_debug_effects_system(r);
    update_ui_raws_system(r);
    update_ui_hierarchy_system(r);
    update_ui_collisions_system(r);
  }

  update_ui_blur_system(r, dt);

#if defined(_DEBUG)
  // hack: reload RAWS
  // const auto& input = get_first_component<SINGLE_InputComponent>(r);
  // if (get_key_down(input, SDL_SCANCODE_9)) {
  //   SDL_Log("%s", std::format("reloading raws...").c_str());
  //   destroy_first<Raws>(r);
  //   create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  //   move_to_scene_start(r, Scene::menu);
  // }
#endif

  // draw a custom mouse cursor
  if (custom_mouse_cursor) {
    // const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    // const auto half_wh = ImVec2{ 0.5f * ri.viewport_size_render_at.x, 0.5f * ri.viewport_size_render_at.y };
    // const auto pos = ImVec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
    // ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    // const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
    // const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
    // ImVec2 tl{ 0.0f, 0.0f };
    // ImVec2 br{ 1.0f, 1.0f };
    // const auto result = convert_sprite_to_uv(r, "CURSOR_0");
    // std::tie(tl, br) = result;
    // const auto size = ImVec2{ 32, 32 };
    // const ImVec2 cursor_tl{ pos.x - (size.x / 2.0f), pos.y - (size.y / 2.0f) };
    // const ImVec2 cursor_br{ pos.x + (size.x / 2.0f), pos.y + (size.y / 2.0f) };
    // draw_list->AddImage(im_id, cursor_tl, cursor_br, tl, br);
  }

  update_render_system(r, dt, mouse_pos);

#if defined(_DEBUG)
  // auto& ri_c = get_first_component<SINGLE_RendererInfo>(r);
  // ImGui::Begin("RenderCalls");
  // ImGui::Text("DrawCalls: %i", ri_c.renderer.draw_calls());
  // ImGui::End();
#endif

  end_frame_render_system(r);
};

} // namespace game2d