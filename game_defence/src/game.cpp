#include "game.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/audio/audio_system.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/system.hpp"
#include "engine/lifecycle/system.hpp"
#include "engine/physics/system.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "game_state.hpp"
#include "modules/actor_breach_charge/breach_charge_system.hpp"
#include "modules/actor_player/system.hpp"
#include "modules/animations/wiggle/wiggle_up_and_down.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/camera/system.hpp"
#include "modules/combat_gun_follow_player/gun_follow_player_system.hpp"
#include "modules/combat_gun_z_index/system.hpp"
#include "modules/combat_show_tiles_in_range/show_tiles_in_range_system.hpp"
#include "modules/events/events_system.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/system.hpp"
#include "modules/system_cooldown/system.hpp"
#include "modules/system_distance_check/system.hpp"
#include "modules/system_entered_new_room/system.hpp"
#include "modules/system_go_from_jetpack_to_dungeon/system.hpp"
#include "modules/system_move_player_on_map/system.hpp"
#include "modules/system_move_to_target_via_lerp/system.hpp"
#include "modules/system_particles/system.hpp"
#include "modules/system_particles_on_death/system.hpp"
#include "modules/system_physics_apply_force/system.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/system_quips/system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_collisions/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_fps_counter/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_input/ui_input_system.hpp"
#include "modules/ui_inventory/ui_inventory_system.hpp"
#include "modules/ui_lootbag/system.hpp"
#include "modules/ui_overworld_boardship/system.hpp"
#include "modules/ui_overworld_shiplabel/system.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_players/ui_players_system.hpp"
#include "modules/ui_raws/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_spaceship_designer/system.hpp"
#include "modules/ui_worldspace_text/system.hpp"
#include "resources/resources.hpp"

#include <fmt/core.h>

namespace game2d {
using namespace std::literals;

void
init(engine::SINGLE_Application& app, entt::registry& r)
{
  init_events_system(r);

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

  create_persistent<SINGLE_QuipsComponent>(r, get_default_quips());
  create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));

  {
    create_persistent<SINGLE_FixedUpdateInputHistory>(r);
    init_input_system(r);
  }

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

  // destroy/create objects
  update_lifecycle_system(r, milliseconds_dt);
  update_physics_apply_force_system(r);
  update_physics_system(r, milliseconds_dt);
  update_player_controller_system(r, milliseconds_dt); // input => actions
  update_events_system(r);                             // dispatch events

  fixed_input.fixed_tick += 1;
};

void
update(engine::SINGLE_Application& app, entt::registry& r, const uint64_t milliseconds_dt)
{
  const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  const float dt = milliseconds_dt / 1000.0f;
  const auto mouse_pos = mouse_position_in_worldspace(r);

  update_input_system(app, r); // sets update_since_last_fixed_update
  update_camera_system(r, dt);
  update_audio_system(r);
  update_events_system(r); // dispatch events

  auto& state = get_first_component<SINGLE_GameStateComponent>(r);
  if (state.state != GameState::PAUSED) {
    update_cooldown_system(r, milliseconds_dt);
    update_move_to_target_via_lerp(r, dt);
    update_particle_system(r, dt);
    update_distance_check_system(r);
    update_entered_new_room_system(r, dt);
    update_wiggle_up_and_down_system(r, dt);
    update_quips_system(r);
    // combat systems
    update_show_tiles_in_range_system(r);
    update_gun_follow_player_system(r, mouse_pos, dt);
    update_gun_z_index_system(r);
    update_go_from_jetpack_to_dungeon_system(r);
    update_spawn_particles_on_death_system(r);
    update_breach_charge_system(r, mouse_pos, dt);
    // movement systems
    update_move_player_on_map_system(r);
#if defined(_DEBUG)
    // update_debug_map_system(r);
#endif
    // update_change_gun_colour_system(r);
    // update_flash_sprite_system(r, milliseconds_dt);
    // update_combat_scale_on_hit_system(r, dt);
    // update_combat_heal_system(r);
    // update_combat_defence_system(r);
    // update_actor_cover_system(r);
    // update_gen_dungeons_system(r, mouse_pos);
    // update_turnbased_endturn_system(r);
    // update_turnbased_enemy_system(r);
    // update_ux_hoverable(r, mouse_pos);
    // update_screenshake_system(r, app.ms_since_launch / 1000.0f, dt);
    // update_fov_system(r, mouse_pos);
    // update_dungeon_helmet_system(r);
    // update_hide_sprites_when_outside_ship_system(r);
    // update_breached_room_system(r);
    //

    if (scene.s == Scene::splashscreen) {
      update_scene_splashscreen_move_to_menu_system(r, dt);
    }

    if (scene.s == Scene::dungeon_designer) {
    }
  }

  // update_animator_system(r, dt);
  update_render_system(r, dt, mouse_pos);

  update_ui_fps_counter_system(r);
  update_ui_raws_system(r);
  update_ui_pause_menu_system(app, r);
  update_ui_inventory_system(r);
  update_ui_lootbag_system(r);
  update_ui_overworld_shiplabel_system(r);
  update_ui_overworld_boardship_system(r);
  update_ui_worldspace_text_system(r);
  // update_ui_gameover_system(r);
  // update_ui_event_console_system(r);
  // update_ui_combat_turnbased_system(r, mouse_pos);
  // update_ui_combat_endturn_system(r);
  // update_ui_combat_ended_system(r);
  // update_ui_combat_info_in_worldspace_system(r);
  // update_ui_launch_crew_system(r);

  if (scene.s == Scene::menu)
    update_ui_scene_main_menu(app, r);

  if (scene.s == Scene::dungeon_designer) {
    update_ui_spaceship_designer_system(r, mouse_pos, dt);
    update_ui_players_system(r);
  }

  static bool show_settings_ui = true;
  if (show_settings_ui) {
    ImGui::ShowDemoWindow(NULL);
    update_ui_hierarchy_system(r);
    update_ui_audio_system(r);
    update_ui_collisions_system(r);
    update_ui_controller_system(r);
    update_ui_input_system(r);
  }

#if defined(_DEBUG)
  // hack: reload RAWS
  // note: this doesnt update anything already spawned from raws data
  // const auto& input = get_first_component<SINGLE_InputComponent>(r);
  // if (get_key_down(input, SDL_SCANCODE_9)) {
  //   fmt::println("reloading raws...");
  //   destroy_first<Raws>(r);
  //   create_persistent<Raws>(r, load_raws("assets/raws/items.jsonc"));
  // }
#endif

  end_frame_render_system(r);
};

} // namespace game2d