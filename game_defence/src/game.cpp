#include "game.hpp"

#include "app/io.hpp"
#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "io/file.hpp"
#include "io/path.hpp"
#include "lifecycle/system.hpp"
#include "modules/actor_breach_charge/system.hpp"
#include "modules/actor_player/system.hpp"
#include "modules/actor_weapon_shotgun/system.hpp"
#include "modules/animation/wiggle_up_and_down.hpp"
#include "modules/animator/system.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/camera/system.hpp"
#include "modules/combat_attack_cooldown/system.hpp"
#include "modules/combat_damage/system.hpp"
#include "modules/combat_heal/system.hpp"
#include "modules/debug_map/system.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/system.hpp"
#include "modules/gen_dungeons/system.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/system.hpp"
#include "modules/screenshake/system.hpp"
#include "modules/system_change_gun_colour/system.hpp"
#include "modules/system_change_gun_z_index/system.hpp"
#include "modules/system_distance_check/system.hpp"
#include "modules/system_entered_new_room/system.hpp"
#include "modules/system_fov/system.hpp"
#include "modules/system_go_from_jetpack_to_dungeon/system.hpp"
#include "modules/system_minigame_bamboo/system.hpp"
#include "modules/system_move_to_target_via_lerp/system.hpp"
#include "modules/system_overworld_change_direction/system.hpp"
#include "modules/system_overworld_fake_fight/system.hpp"
#include "modules/system_particles/system.hpp"
#include "modules/system_particles_on_death/system.hpp"
#include "modules/system_physics_apply_force/system.hpp"
#include "modules/system_quips/components.hpp"
#include "modules/system_quips/system.hpp"
#include "modules/system_turnbased_endturn/system.hpp"
#include "modules/system_turnbased_enemy/system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_collisions/system.hpp"
#include "modules/ui_colours/system.hpp"
#include "modules/ui_combat_ended/system.hpp"
#include "modules/ui_combat_endturn/system.hpp"
#include "modules/ui_combat_info_in_worldspace/system.hpp"
#include "modules/ui_combat_turnbased/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_event_console/system.hpp"
#include "modules/ui_gameover/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_inventory/system.hpp"
#include "modules/ui_overworld_launch_crew/system.hpp"
#include "modules/ui_overworld_ship_label/system.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_worldspace_sprite/system.hpp"
#include "modules/ui_worldspace_text/system.hpp"
#include "modules/ux_hoverable/system.hpp"
#include "modules/ux_selectable_by_keyboard/system.hpp"
#include "physics/system.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "fmt/core.h"
#include "imgui.h"

#if !defined(__EMSCRIPTEN__)
// #include "optick.h"
#else
constexpr auto OPTICK_EVENT = [](const std::string& str) {}; // do nothing
constexpr auto OPTICK_FRAME = [](const std::string& str) {}; // do nothing
#endif

#include <algorithm>

namespace game2d {
using namespace std::literals;

void
init(engine::SINGLETON_Application& app, entt::registry& r)
{
  const auto start = std::chrono::high_resolution_clock::now();

  // Fonts
  auto& io = ImGui::GetIO();
  float font_scale = 14.0f;
  //   font_scale = X.0f; // 4k scale?
  const std::string exe_path = engine::get_exe_path_without_exe_name();
  fmt::println("exe path: {}", exe_path);
  const std::string font_path = exe_path + "assets/fonts/Roboto-Regular.ttf"s;
  io.Fonts->AddFontFromFileTTF(font_path.c_str(), font_scale);

  {
    SINGLETON_RendererInfo ri;

    std::string path = engine::get_exe_path_without_exe_name();
    path += "assets/";

    Texture kennynl;
    kennynl.path = path + "textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
    kennynl.spritesheet_path = path + "config/spritemap_kennynl.json";
    ri.user_textures.push_back(kennynl);

    Texture gameicons;
    gameicons.path = path + "textures/kennynl_gameicons/Spritesheet/sheet_white1x_adjusted.png";
    gameicons.spritesheet_path = path + "config/spritemap_kennynl_icons.json";
    ri.user_textures.push_back(gameicons);

    Texture spacestation_0;
    spacestation_0.path = path + "textures/spacestation_0.png";
    spacestation_0.spritesheet_path = path + "config/spritemap_spacestation_0.json";
    ri.user_textures.push_back(spacestation_0);

    Texture studio_logo;
    studio_logo.path = path + "textures/blueberry-dark.png";
    studio_logo.spritesheet_path = path + "config/spritemap_studio_logo.json";
    ri.user_textures.push_back(studio_logo);

    Texture custom;
    custom.path = path + "textures/custom.png";
    custom.spritesheet_path = path + "config/spritemap_custom.json";
    ri.user_textures.push_back(custom);

    // load spritesheet info
    SINGLE_Animations anims;
    for (const auto& tex : ri.user_textures)
      load_sprites(anims, tex.spritesheet_path);

    create_empty<SINGLE_Animations>(r, anims);
    create_empty<SINGLETON_RendererInfo>(r, ri);
  }

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  init_render_system(app, r, ri); // init after camera

  create_empty<SINGLETON_FixedUpdateInputHistory>(r);
  init_input_system(r);

  init_ui_colour_palette_system(r);

  // Load Quips
  const auto quips = load_file_into_lines("assets/writing/quips.txt");
  const auto quips_hit = load_file_into_lines("assets/writing/quips_hit.txt");
  const auto quips_encounter = load_file_into_lines("assets/writing/quips_encounter.txt");
  SINGLE_QuipsComponent quips_c;

  quips_c.quips = quips;
  quips_c.quips_unused = quips;

  quips_c.quips_encounter = quips_encounter;
  quips_c.quips_encounter_unused = quips_encounter;

  quips_c.quips_hit = quips_hit;
  quips_c.quips_hit_unused = quips_hit;

  create_empty<SINGLE_QuipsComponent>(r, quips_c);

  // move to splash screen before doing any heavy lifting (i.e. app freezing)
  engine::log_time_since("Moving to splash screen.", start);
  move_to_scene_start(r, Scene::splashscreen);
}

// i.e. do things heavy during splash screen.
void
init_slow(engine::SINGLETON_Application& app, entt::registry& r)
{
  {
    std::string path = engine::get_exe_path_without_exe_name();
    path += "assets/audio/";

    SINGLETON_AudioComponent audio;

    audio.sounds.push_back(
      { "SHOTGUN_SHOOT_01", path + "FIREARM_Shotgun_Model_02_Fire_Single_RR1_stereo.wav", SoundType::SFX });

    audio.sounds.push_back({ "SHOTGUN_RELOAD_01", path + "RELOAD_Pump_stereo.wav", SoundType::SFX });
    audio.sounds.push_back({ "TAKE_DAMAGE_01", path + "GRUNT_Male_Subtle_Hurt_mono.wav", SoundType::SFX });

    audio.sounds.push_back({ "MENU_01", path + "scott-buckley-moonlight.mp3", SoundType::BACKGROUND });
    audio.sounds.push_back({ "GAME_01", path + "alex-productions-arnor.mp3", SoundType::BACKGROUND });
    // audio.sounds.push_back({ "GAME_01", "purrple-cat-green-tea.mp3" });
    // audio.sounds.push_back({ "MENU_01", "scott-buckley-phaseshift.mp3" });
    // audio.sounds.push_back({ "COMBAT_01", path + ".mp3", SoundType::BACKGROUND });
    // audio.sounds.push_back({ "WIN_01", "8-bit-win-funk-david-renda.wav" });
    // audio.sounds.push_back({ "LOSS_01", "8-bit-loss-david-renda.wav" });

    // audio.sounds.push_back({ "ENEMY_LOCKON", path + "UI_SCI-FI_Tone_Bright_Wet_16_stereo.wav", SoundType::SFX });
    // audio.sounds.push_back({ "ENEMY_LOCKOFF", path + "UI_SCI-FI_Tone_Bright_Wet_17_stereo.wav", SoundType::SFX });

    destroy_first_and_create<SINGLETON_AudioComponent>(r, audio);
  }
  init_audio_system(r);
}

void
duplicate_held_input(SINGLETON_FixedUpdateInputHistory& fixed_input)
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
fixed_update(engine::SINGLETON_Application& app, entt::registry& game, const uint64_t milliseconds_dt)
{
  // OPTICK_EVENT("FixedUpdate()");

  auto& input = get_first_component<SINGLETON_InputComponent>(game);
  auto& fixed_input = get_first_component<SINGLETON_FixedUpdateInputHistory>(game);

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

  // allow gameover/restart requests to be processed
  update_gameover_system(game);

  auto& state = get_first_component<SINGLETON_GameStateComponent>(game);
  if (state.state == GameState::PAUSED)
    return; // note: this ignores inputs

  auto& gameover = get_first_component<SINGLETON_GameOver>(game);
  if (gameover.game_is_over)
    return;

  // destroy/create objects
  update_lifecycle_system(game, milliseconds_dt);

  {
    // OPTICK_EVENT("(physics-tick)");
    update_physics_apply_force_system(game);
    update_physics_system(game, milliseconds_dt);
  }

  {
    // OPTICK_EVENT("fixed-game-tick");
    update_resolve_collisions_system(game);

    // put immediately after collisions,
    // otherwise the DealDamageRequest entity could be removed
    update_take_damage_system(game);

    update_player_controller_system(game, milliseconds_dt); // input => actions
  }

  fixed_input.fixed_tick += 1;
}

void
update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  // OPTICK_EVENT("(update)");
  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);

  update_input_system(app, r); // sets update_since_last_fixed_update

  // After update_input_system
  const glm::ivec2 mouse_pos = mouse_position_in_worldspace(r);

  {
    // OPTICK_EVENT("(update)-game-tick");

    // core
    update_camera_system(r, dt);
    update_audio_system(r);
    // update_cursor_system(r, mouse_pos);

    // systems
    const auto& state = get_first_component<SINGLETON_GameStateComponent>(r);
    auto& gameover = get_first_component<SINGLETON_GameOver>(r);

    const std::vector<Scene> invalid_scenes{
      // dont do game systems these scenes
      Scene::splashscreen,
    };
    const bool in_invalid_scene = std::find(invalid_scenes.begin(), invalid_scenes.end(), scene.s) != invalid_scenes.end();
    if (!in_invalid_scene && state.state == GameState::RUNNING && !gameover.game_is_over) {

      // Only keeping this here until I'm convinced that
      // putting all these systems in update isn't a mistake
      const uint64_t milliseconds_dt = static_cast<uint64_t>(dt * 1000.0f);

      // potentially common
      update_attack_cooldown_system(r, milliseconds_dt);
      update_change_gun_colour_system(r);
      update_change_gun_z_index_system(r);
      update_distance_check_system(r);
      update_move_to_target_via_lerp(r, dt);
      update_particle_system(r, dt);
      update_quips_system(r);
      update_spawn_particles_on_death_system(r);
      update_weapon_shotgun_system(r, milliseconds_dt);
      update_wiggle_up_and_down_system(r, dt);
    }

    if (scene.s == Scene::overworld_revamped) {
      update_overworld_change_direction_system(r);
      // update_overworld_fake_fight_system(r);
    }

    if (scene.s == Scene::dungeon_designer) {
      update_entered_new_room_system(r, dt);
      update_gen_dungeons_system(r, mouse_pos);
      update_turnbased_endturn_system(r);
      update_turnbased_enemy_system(r);
      update_ux_hoverable(r, mouse_pos);
      update_ux_selectable_by_keyboard_system(r);
      update_screenshake_system(r, app.ms_since_launch / 1000.0f, dt);
      update_fov_system(r, mouse_pos);
      update_combat_heal_system(r);
      update_breach_charge_system(r, mouse_pos, dt);
      update_go_from_jetpack_to_dungeon_system(r);

#if defined(_DEBUG)
      update_debug_map_system(r);
#endif
    }

    if (scene.s == Scene::splashscreen)
      update_scene_splashscreen_move_to_menu_system(r, dt);

    if (scene.s == Scene::minigame_bamboo) {
      update_minigame_bamboo_system(r, dt);
    }
  }

  {
    // OPTICK_EVENT("(update)-update-render-system");
    update_animator_system(r, dt);
    update_render_system(r, dt, mouse_pos);
  }

  {
    // OPTICK_EVENT("(update)-update-ui");

    // Display a parented viewport window at the top of the screen, that shows the fps.
    const bool show_fps_counter = true;
    if (show_fps_counter) {
      const std::string example = "FPS: 10000.00";

      ImGuiWindowFlags flags = 0;
      // position and sizing
      flags |= ImGuiWindowFlags_NoDecoration;
      flags |= ImGuiWindowFlags_NoMove;
      flags |= ImGuiWindowFlags_NoBackground;
      flags |= ImGuiWindowFlags_NoDocking;
      flags |= ImGuiWindowFlags_NoSavedSettings;
      flags |= ImGuiWindowFlags_NoFocusOnAppearing;
      // flags |= ImGuiWindowFlags_NoNav;

      ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0, 0 });

      ImGui::Begin("FPS", NULL, flags);
      ImGui::Text("FPS: %0.2f", ImGui::GetIO().Framerate);
      ImGui::End();
    }

#if defined(_DEBUG)
    ImGui::ShowDemoWindow(NULL);
#endif

    if (scene.s == Scene::menu) {
      update_ui_scene_main_menu(app, r);
    }
    if (scene.s == Scene::dungeon_designer) {
      update_ui_combat_turnbased_system(r, mouse_pos);
      update_ui_combat_endturn_system(r);
      update_ui_combat_ended_system(r);
      update_ui_inventory_system(r);
      update_ui_combat_info_in_worldspace_system(r);
    }
    if (scene.s == Scene::overworld_revamped) {
      update_ui_launch_crew_system(r);
    }
    update_ui_overworld_shiplabel_system(r);
    update_ui_worldspace_text_system(r);
    update_ui_worldspace_sprite_system(r);
    update_ui_pause_menu_system(app, r);
    update_ui_gameover_system(r);
    update_ui_event_console_system(r);

#if defined(_DEBUG)
    // todo: put in to a settings menu
    static bool show_settings_ui = true;
    if (show_settings_ui) {
      update_ui_audio_system(r);
      update_ui_controller_system(r);
      update_ui_hierarchy_system(r);
      update_ui_collisions_system(r);
    }
#endif
  }

  end_frame_render_system(r);
};

} // namespace game2d