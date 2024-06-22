#include "game.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/system.hpp"
#include "modules/actor_cursor/system.hpp"
#include "modules/actor_enemy_patrol/system.hpp"
#include "modules/actor_player/system.hpp"
#include "modules/actor_spawner/system.hpp"
#include "modules/actor_weapon_shotgun/system.hpp"
#include "modules/animation/angle_to_velocity.hpp"
#include "modules/animation/wiggle_up_and_down.hpp"
#include "modules/animator/system.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/camera/system.hpp"
#include "modules/combat_attack_cooldown/system.hpp"
#include "modules/combat_damage/system.hpp"
#include "modules/combat_flash_on_damage/system.hpp"
#include "modules/combat_wants_to_shoot/system.hpp"
#include "modules/debug_pathfinding/system.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/system.hpp"
#include "modules/gen_dungeons/system.hpp"
#include "modules/lerp_to_target/system.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/scene_splashscreen_move_to_menu/system.hpp"
#include "modules/screenshake/system.hpp"
#include "modules/system_minigame_bamboo/system.hpp"
#include "modules/system_particles/system.hpp"
#include "modules/system_particles_on_death/system.hpp"
#include "modules/system_pathfinding/system.hpp"
#include "modules/system_sprint/system.hpp"
#include "modules/system_turnbased_endturn/system.hpp"
#include "modules/system_turnbased_enemy/system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_collisions/system.hpp"
#include "modules/ui_colours/system.hpp"
#include "modules/ui_combat_turnbased/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_gameover/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_patrol/system.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_worldspace_text/system.hpp"
#include "modules/ux_hoverable/system.hpp"
#include "modules/ux_hoverable_change_colour/system.hpp"
#include "physics/components.hpp"
#include "physics/process_actor_actor_collisions.hpp"
#include "physics/process_move_objects.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "imgui.h"
#include "optick.h"

#include <algorithm>
#include <ranges>

namespace game2d {
using namespace std::literals;

void
init(engine::SINGLETON_Application& app, entt::registry& r)
{
  {
    SINGLETON_AudioComponent audio;
    audio.sounds.push_back({ "SHOTGUN_SHOOT_01", "assets/audio/FIREARM_Shotgun_Model_02_Fire_Single_RR1_stereo.wav" });
    audio.sounds.push_back({ "SHOTGUN_SHOOT_02", "assets/audio/FIREARM_Shotgun_Model_02_Fire_Single_RR2_stereo.wav" });
    audio.sounds.push_back({ "SHOTGUN_SHOOT_03", "assets/audio/FIREARM_Shotgun_Model_02_Fire_Single_RR3_stereo.wav" });
    audio.sounds.push_back({ "SHOTGUN_RELOAD_01", "assets/audio/RELOAD_Pump_stereo.wav" });
    audio.sounds.push_back({ "MENU_01", "assets/audio/8-bit-menu-david-renda.wav" });
    audio.sounds.push_back({ "GAME_01", "assets/audio/8-bit-adventure-david-renda.wav" });
    audio.sounds.push_back({ "WIN_01", "assets/audio/8-bit-win-funk-david-renda.wav" });
    audio.sounds.push_back({ "LOSS_01", "assets/audio/8-bit-loss-david-renda.wav" });
    audio.sounds.push_back({ "TAKE_DAMAGE_01", "assets/audio/GRUNT_Male_Subtle_Hurt_mono.wav" });

    create_empty<SINGLETON_AudioComponent>(r, audio);
  }
  init_audio_system(r);

  {
    SINGLETON_RendererInfo ri;

    Texture kennynl;
    kennynl.path = "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
    kennynl.spritesheet_path = "assets/config/spritemap_kennynl.json";
    ri.user_textures.push_back(kennynl);

    Texture gameicons;
    gameicons.path = "assets/textures/kennynl_gameicons/Spritesheet/sheet_white1x_adjusted.png";
    gameicons.spritesheet_path = "assets/config/spritemap_kennynl_icons.json";
    ri.user_textures.push_back(gameicons);

    Texture space_background;
    space_background.path = "assets/textures/space_background_stars_only.png";
    space_background.spritesheet_path = "assets/config/spritemap_space_background_0.json";
    ri.user_textures.push_back(space_background);

    Texture mainmenu_background;
    mainmenu_background.path = "assets/textures/background_mainmenu.png";
    mainmenu_background.spritesheet_path = "assets/config/spritemap_space_background_0.json"; // lazy
    ri.user_textures.push_back(mainmenu_background);

    Texture spacestation_0;
    spacestation_0.path = "assets/textures/spacestation_0.png";
    spacestation_0.spritesheet_path = "assets/config/spritemap_spacestation_0.json";
    ri.user_textures.push_back(spacestation_0);

    Texture studio_logo;
    studio_logo.path = "assets/textures/blueberry.png";
    studio_logo.spritesheet_path = "assets/config/spritemap_studio_logo.json";
    ri.user_textures.push_back(studio_logo);

    // load spritesheet info
    SINGLE_Animations anims;
    for (const auto& tex : ri.user_textures)
      load_sprites(anims, tex.spritesheet_path);

    create_empty<SINGLE_Animations>(r, anims);
    create_empty<SINGLETON_RendererInfo>(r, ri);
  }

  init_ui_colour_palette_system(r);

  // add camera
  OrthographicCamera camera_info;
  camera_info.projection = calculate_ortho_projection(app.width, app.height);
  const auto camera_e = create_empty<OrthographicCamera>(r, camera_info);
  r.emplace<TransformComponent>(camera_e);

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  init_render_system(app, r, ri); // init after camera

  create_empty<SINGLETON_FixedUpdateInputHistory>(r);

  move_to_scene_start(r, Scene::splashscreen);
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

} // namespace game2d

void
game2d::fixed_update(engine::SINGLETON_Application& app, entt::registry& game, const uint64_t milliseconds_dt)
{
  OPTICK_EVENT("FixedUpdate()");

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
    OPTICK_EVENT("(physics-tick)");
    auto& physics = get_first_component<SINGLETON_PhysicsComponent>(game);
    physics.frame_collisions.clear();

    // todo: split out updating aabb from move_objects system
    update_move_objects_system(game, milliseconds_dt);
    update_actor_actor_collisions_system(game, physics);
  }

  {
    OPTICK_EVENT("fixed-game-tick");
    update_resolve_collisions_system(game);

    // put immediately after collisions,
    // otherwise the DealDamageRequest entity could be removed
    update_take_damage_system(game);

    update_player_controller_system(game, milliseconds_dt); // input => actions
  }

  fixed_input.fixed_tick += 1;
}

void
game2d::update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  OPTICK_EVENT("(update)");
  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);

  update_input_system(app, r); // sets update_since_last_fixed_update

  // After update_input_system
  const glm::ivec2 mouse_pos = mouse_position_in_worldspace(r);

  {
    OPTICK_EVENT("(update)-game-tick");

    // core
    update_camera_system(r, dt, mouse_pos);
    update_audio_system(r);
    update_cursor_system(r, mouse_pos);

    // systems
    update_screenshake_system(r, app.ms_since_launch / 1000.0f, dt);
    update_ux_hoverable(r);
    update_ux_hoverable_change_colour_system(r);

    const auto& state = get_first_component<SINGLETON_GameStateComponent>(r);
    auto& gameover = get_first_component<SINGLETON_GameOver>(r);

    const std::vector<Scene> invalid_scenes{
      // dont do game systems these scenes
      Scene::splashscreen,
      Scene::menu,
    };
    const bool in_invalid_scene = std::find(invalid_scenes.begin(), invalid_scenes.end(), scene.s) != invalid_scenes.end();
    if (!in_invalid_scene && state.state == GameState::RUNNING && !gameover.game_is_over) {

      // Only keeping this here until I'm convinced that
      // putting all these systems in update isn't a mistake
      const uint64_t milliseconds_dt = static_cast<uint64_t>(dt * 1000.0f);

      // REMOVED SYSTEMS... for one reason or another.
      //
      // update_wiggle_up_and_down_system(r, dt);
      //
      // update_sprite_spritestack_system(r, dt);
      // update_actor_bodypart_head_system(r, dt, mouse_pos);
      // update_actor_bodypart_legs_system(r, dt, mouse_pos);
      // update_combat_powerup_doubledamage_system(r, dt);
      // update_actor_group_system(r, mouse_pos);
      // update_spaceship_door_system(r, dt);
      // update_intent_pickup_system(r);
      // update_intent_drop_item_system(r);
      //   update_ui_inventory(r);
      //   update_ui_selected(r);
      //   update_ui_arrows_to_spawners_system(r);
      //   update_ui_player_name_above_player_system(r);
      //   update_ui_level_up_system(r);
      //   update_ui_xp_bar_system(r);
      //   update_ui_rpg_character_system(r);
      //   update_ui_inverse_kinematics_system(r, mouse_pos);
      //

      // potentially common
      update_attack_cooldown_system(r, milliseconds_dt);
      update_flash_sprite_system(r, milliseconds_dt);
      update_pathfinding_system(r, dt);
      update_particle_system(r, dt);
      update_spawn_particles_on_death_system(r);
      update_set_velocity_to_target_system(r, dt);
      update_spawner_system(r, milliseconds_dt);
      update_sprint_system(r, dt);
      update_wants_to_shoot_system(r);
      update_weapon_shotgun_system(r, milliseconds_dt);
    }

    if (scene.s == Scene::overworld) {
      update_scale_by_velocity_system(r, dt);
      update_actor_enemy_patrol_system(r, mouse_pos, dt);
    }

    if (scene.s == Scene::dungeon_designer || scene.s == Scene::turnbasedcombat) {
      update_gen_dungeons_system(r, mouse_pos);
      update_turnbased_endturn_system(r);
      update_turnbased_enemy_system(r);

#if defined(_DEBUG)
      // update_debug_pathfinding_system(r, mouse_pos);
#endif
    }

    if (scene.s == Scene::splashscreen)
      update_scene_splashscreen_move_to_menu_system(r, dt);

    if (scene.s == Scene::minigame_bamboo) {
      update_minigame_bamboo_system(r, dt);
    }
  }

  {
    OPTICK_EVENT("(update)-update-render-system");
    update_animator_system(r, dt);
    update_render_system(r, dt, mouse_pos);
  }

  {
    OPTICK_EVENT("(update)-update-ui");

    // Display a parented viewport window at the top of the screen, that shows the fps.
    const bool show_fps_counter = true;
    if (show_fps_counter) {
      const std::string example = "FPS: 10000.00";
      const float size_y = ImGui::CalcTextSize(example.c_str()).y / 2.0f;

      ImGuiWindowFlags flags = 0;
      static bool menubar_open = true;
      flags |= ImGuiWindowFlags_NoFocusOnAppearing;
      flags |= ImGuiWindowFlags_NoCollapse;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiDockNodeFlags_NoResize;
      ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0, 0 });
      ImGui::SetNextWindowSize({ 100, size_y }, ImGuiCond_Always);

      const ImGuiID dockspace_id = ImGui::GetID("RootDockSpace");
      ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_Always);
      ImGui::Begin("MenuBar", &menubar_open, flags);
      ImGui::Text("FPS: %0.2f", ImGui::GetIO().Framerate);
      ImGui::End();
    }

    if (scene.s == Scene::menu) {
      update_ui_scene_main_menu(app, r);
    }
    if (scene.s == Scene::dungeon_designer) {
      update_ui_combat_turnbased_system(r, mouse_pos);
    }
    if (scene.s == Scene::overworld) {
      update_ui_patrol_system(r);
    }
    if (scene.s == Scene::turnbasedcombat) {
      update_ui_combat_turnbased_system(r, mouse_pos);
    }

    update_ui_worldspace_text_system(r);
    update_ui_pause_menu_system(app, r);
    update_ui_gameover_system(r);

    // todo: put in to a settings menu
    static bool show_settings_ui = true;
    if (show_settings_ui) {
      update_ui_audio_system(r);
      update_ui_controller_system(r);
      update_ui_hierarchy_system(r);
      update_ui_collisions_system(r);
    }
  }

  end_frame_render_system(r);
};
