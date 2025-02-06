#include "game.hpp"

#include "engine/audio/audio_system.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/system.hpp"
#include "engine/lifecycle/lifecycle_system.hpp"
#include "engine/physics/physics_system.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actor_player/actor_player_system.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_system.hpp"
#include "modules/combat_scale_on_hit/combat_scale_on_hit_system.hpp"
#include "modules/controller_input_open_ui/controller_input_open_ui_system.hpp"
#include "modules/core_animations/rotate_system.hpp"
#include "modules/core_animations/wiggle/wiggle_up_and_down.hpp"
#include "modules/core_animator/animator_system.hpp"
#include "modules/core_camera/camera_system.hpp"
#include "modules/core_camera/helpers.hpp"
#include "modules/core_camera/orthographic.hpp"
#include "modules/core_events/events_system.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/system.hpp"
#include "modules/core_scene/scene_components.hpp"
#include "modules/core_scene/scene_helpers.hpp"
#include "modules/core_sprites/sprite_helpers.hpp"
#include "modules/effect_crt/crt_components.hpp"
#include "modules/steam/steam_helpers.hpp"
#include "modules/steam_debug_ui/steam_debug_ui_system.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_autofire/autofire_system.hpp"
#include "modules/system_cooldown/cooldown_system.hpp"
#include "modules/system_distance_check/system.hpp"
#include "modules/system_enemy_projectile/enemy_projectile_system.hpp"
#include "modules/system_hulls/hardpoints_system.hpp"
#include "modules/system_hulls/hulls_helpers.hpp"
#include "modules/system_manualfire/manualfire_system.hpp"
#include "modules/system_move_to_target_via_lerp/move_to_target_via_lerp_system.hpp"
#include "modules/system_particles/system.hpp"
#include "modules/system_particles_on_death/system.hpp"
#include "modules/system_physics_apply_force/physics_apply_force_system.hpp"
#include "modules/system_scene_splashscreen_move_to_menu/system.hpp"
#include "modules/system_spawner/spawner_system.hpp"
#include "modules/system_spritestack/spritestack_system.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade/upgrade_helpers.hpp"
#include "modules/system_upgrade_max_hp/upgrade_max_hp_system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_collisions/system.hpp"
#include "modules/ui_colours/ui_colours_system.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_system.hpp"
#include "modules/ui_debug_spawner/ui_debug_spawner_system.hpp"
#include "modules/ui_debug_upgrades/ui_debug_upgrades_system.hpp"
#include "modules/ui_fps_counter/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_imgui_colours/ui_imgui_colours.hpp"
#include "modules/ui_popup_options/ui_popup_options_system.hpp"
#include "modules/ui_popup_pause/ui_popup_pause_system.hpp"
#include "modules/ui_raws/ui_raws_system.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_system.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_system.hpp"
#include "modules/ui_scene_select/scene_select_system.hpp"
#include "modules/ui_scene_survive/scene_survive_system.hpp"
#include "modules/ui_scene_survive_health/ui_survive_health_system.hpp"
#include "modules/ui_scene_survive_level_up/ui_survive_level_up_components.hpp"
#include "modules/ui_scene_survive_level_up/ui_survive_level_up_system.hpp"
#include "modules/ui_scene_survive_timer/ui_survive_timer_system.hpp"
#include "modules/ui_scene_survive_xp_bar/ui_survive_xp_bar_system.hpp"
#include "modules/ui_sdl2_controller/ui_sdl2_controller_system.hpp"
#include "modules/ui_sdl2_input/ui_sdl2_input_system.hpp"
#include "resources/resources.hpp"

#include <SDL2/SDL_log.h>
#include <imgui.h>
#include <steam/steam_api.h>
#include <steam/steam_api_common.h>

#if defined(_MSC_VER)
#include <optick.h>
#endif

namespace game2d {
using namespace std::literals;

bool custom_mouse_cursor = false;

bool
require_pause(entt::registry& r)
{
  bool pause = false;

  // pause due to needing level up
  auto lv_up_e = get_first<SINGLE_LevelUpUI>(r);
  if (lv_up_e != entt::null) {
    const auto& lv_up_c = r.get<SINGLE_LevelUpUI>(lv_up_e);
    pause |= lv_up_c.require_level_up;
  }

  return pause;
}

void
init(engine::SINGLE_Application& app, entt::registry& r)
{
  init_events_system(r);

  // Fonts
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontDefault();

  // font for survive timer
  ImFontConfig fontConfig;
  fontConfig.PixelSnapH = true;
  io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 32.0f, &fontConfig);

  // font for countdown timer
  ImFontConfig countdown_config;
  countdown_config.PixelSnapH = true;
  io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Medium.ttf", 128.0f, &countdown_config);

  // hide default cursor
  if (custom_mouse_cursor) {
    // io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    // auto result = SDL_ShowCursor(SDL_DISABLE);
    // if (result < 0)
    //   SDL_Log("Failed to hide system cursor: %s", SDL_GetError());
  }

  {
    SINGLE_RendererInfo ri = get_default_textures();
    SINGLE_Animations anims;
    for (const auto& tex : ri.user_textures)
      load_sprites(anims, tex.spritesheet_path);
    create_persistent<SINGLE_Animations>(r, anims);
    create_persistent<SINGLE_RendererInfo>(r, ri);
    create_persistent<OrthographicCamera>(r);
    r.emplace<TransformComponent>(get_first<OrthographicCamera>(r));
    init_render_system(app, r);
  }

  create_persistent<SINGLE_DebugMenuBar>(r);
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  create_persistent<SINGLE_Hulls>(r, load_hulls("assets/raws/hulls/"));
  create_persistent<SINGLE_Upgrades>(r, load_upgrades("assets/raws/upgrades.jsonc"));
  create_persistent<SINGLE_EffectCrt>(r);

  create_persistent<SINGLE_FixedUpdateInputHistory>(r);
  init_input_system(r);
  init_steam(r);
  init_steam_input(r);
  create_persistent<SteamOverlayManager>(r);
  create_persistent<SINGLE_SteamControllerGameState>(r);

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
#if defined(_MSC_VER)
  OPTICK_EVENT();
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
  fixed_update_player_controller_system(r, milliseconds_dt, mouse_pos);

  // fixed_input.fixed_tick += 1;
};

void
update(engine::SINGLE_Application& app, entt::registry& r, const uint64_t milliseconds_dt)
{
#if defined(_MSC_VER)
  OPTICK_EVENT();
#endif

  const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  const float dt = milliseconds_dt / 1000.0f;
  const auto mouse_pos = mouse_position_in_worldspace(r);

  begin_frame_sprite(r);

  SteamAPI_RunCallbacks();

  update_sdl_event_system(app, r); // sets update_since_last_fixed_update
  update_steam_input(r);
  update_controller_input_open_ui_system(r);
  update_camera_system(r, dt);
  update_audio_system(r);
  update_events_system(r); // dispatch events
  update_player_controller_system(r, milliseconds_dt, mouse_pos);
  // update_screenshake_system(r, app.ms_since_launch / 1000.0f, dt);

  if (scene.s == Scene::splashscreen)
    update_scene_splashscreen_move_to_menu_system(r, dt);

  // pause due to gamelogic
  bool pause = require_pause(r);

  update_ship_draw_arcs_system(r);
  update_upgrade_max_hp_system(r);

  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (state.state != GameState::PAUSED && !pause) {
    update_animator_system(r, dt);
    update_animation_rotate_system(r, dt);
    update_autofire_system(r, mouse_pos); // prefer after calculating arcs this frame
    update_manualfire_system(r);
    update_combat_scale_on_hit_system(r, dt);
    update_cooldown_system(r, milliseconds_dt);
    update_distance_check_system(r);
    update_gun_follow_player_system(r, mouse_pos, dt);
    update_move_to_target_via_lerp(r, dt);
    update_particle_system(r, dt);
    update_spawn_particles_on_death_system(r);
    update_wiggle_up_and_down_system(r, dt);
    update_spawner_system(r);
    update_enemy_projectile_system(r);
    update_sprite_spritestack_system(r, dt);
  }

#if defined(_DEBUG)
  // update_debug_fixtures_system(r);
#endif

  update_ui_fps_counter_system(r);
  update_ui_popup_pause_system(app, r);
  update_ui_popup_options_system(r);
  // update_ui_worldspace_text_system(r);

  if (scene.s == Scene::menu) {
    update_ui_scene_main_menu(app, r);
    update_ui_scene_main_menu_playerjoin_system(r);
  }

  if (scene.s == Scene::select)
    update_ui_scene_select_system(r, dt);

  if (scene.s == Scene::survive) {
    update_ui_scene_survive_system(r);
    update_ui_survive_timer_system(r);
    update_ui_survive_health_system(r);
    update_ui_survive_xp_bar_system(r);
    update_ui_survive_level_up_system(r);
    // update_ui_combat_damage_numbers_system(r, dt, mouse_pos);
    // update_ui_gameover_system(r);
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

    auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);

    auto demo_state = gesert_menubar_state(menu_c, "ImGui Demo");
    if (demo_state.enabled)
      ImGui::ShowDemoWindow(NULL);

    auto audio_state = gesert_menubar_state(menu_c, "Audio");
    if (audio_state.enabled)
      update_ui_audio_system(r);

    auto sdl2_controller_state = gesert_menubar_state(menu_c, "SDL2 Controller");
    if (sdl2_controller_state.enabled)
      update_ui_controller_system(r);

    auto sdl2_input_state = gesert_menubar_state(menu_c, "SDL2 Input");
    if (sdl2_input_state.enabled)
      update_ui_sdl2_input_system(r);

    auto ui_steam_state = gesert_menubar_state(menu_c, "Steam");
    if (ui_steam_state.enabled)
      update_steam_debug_ui_system(r);

    update_ui_colours_system(r);
    update_ui_debug_spawner_system(r);
    update_ui_raws_system(r);
    update_ui_hierarchy_system(r);
    update_ui_collisions_system(r);
  }

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