#include "game.hpp"

#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "modules/actor_bodypart_head/system.hpp"
#include "modules/actor_bodypart_legs/system.hpp"
#include "modules/actor_cursor/system.hpp"
#include "modules/actor_enemy/system.hpp"
#include "modules/actor_group/system.hpp"
#include "modules/actor_particle/system.hpp"
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
#include "modules/combat_powerup_doubledamage/system.hpp"
#include "modules/combat_wants_to_shoot/system.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/system.hpp"
#include "modules/items_drop/system.hpp"
#include "modules/items_pickup/system.hpp"
#include "modules/lerp_to_target/system.hpp"
#include "modules/lifecycle/system.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/screenshake/system.hpp"
#include "modules/selected_interactions/system.hpp"
#include "modules/sprite_spritestack/system.hpp"
#include "modules/ui_arrows_to_spawners/system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_collisions/system.hpp"
#include "modules/ui_colours/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_gameover/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_inventory/system.hpp"
#include "modules/ui_level_editor/components.hpp"
#include "modules/ui_level_editor/system.hpp"
#include "modules/ui_level_up/components.hpp"
#include "modules/ui_level_up/system.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_player_name_above_player/system.hpp"
#include "modules/ui_rpg_character/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_selected/system.hpp"
#include "modules/ui_worldspace_text/system.hpp"
#include "modules/ui_xp_bar/system.hpp"
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

void
game2d::init(engine::SINGLETON_Application& app, entt::registry& r)
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
    r.emplace<SINGLETON_AudioComponent>(r.create(), audio);
  }
  init_audio_system(r);

  {
    SINGLETON_RendererInfo ri;

    Texture kennynl;
    kennynl.path = "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
    kennynl.spritesheet_path = "assets/config/spritemap_kennynl.json";
    ri.user_textures.push_back(kennynl);

    Texture custom;
    custom.path = "assets/textures/bargame_aseprite.png";
    custom.spritesheet_path = "assets/config/spritemap_custom.json";
    ri.user_textures.push_back(custom);

    Texture particles;
    particles.path = "assets/textures/particles.png";
    particles.spritesheet_path = "assets/config/spritemap_particles.json";
    ri.user_textures.push_back(particles);

    Texture gameicons;
    gameicons.path = "assets/textures/kennynl_gameicons/Spritesheet/sheet_white1x_adjusted.png";
    gameicons.spritesheet_path = "assets/config/spritemap_kennynl_icons.json";
    ri.user_textures.push_back(gameicons);

    Texture gun_pistol;
    gun_pistol.path = "assets/textures/voxel/gun_pistol.png";
    gun_pistol.spritesheet_path = "assets/config/spritemap_voxel_gun_pistol.json";
    ri.user_textures.push_back(gun_pistol);

    // load spritesheet info
    SINGLE_Animations anims;
    for (const auto& tex : ri.user_textures)
      load_sprites(anims, tex.spritesheet_path);
    r.emplace<SINGLE_Animations>(r.create(), anims);

    r.emplace<SINGLETON_RendererInfo>(r.create(), ri);
  }

  r.emplace<SINGLETON_FixedUpdateInputHistory>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  r.emplace<SINGLETON_LevelEditor>(r.create());
  init_ui_colour_palette_system(r);

  const auto camera = r.create();
  r.emplace<TagComponent>(camera, "camera");
  OrthographicCamera camera_info;
  camera_info.projection = calculate_ortho_projection(app.width, app.height);
  r.emplace<OrthographicCamera>(camera, camera_info);
  r.emplace<TransformComponent>(camera);
  // r.emplace<HasTargetPositionComponent>(camera);
  // r.emplace<LerpToTargetComponent>(camera, 0.5f);

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  init_render_system(app, r, ri); // init after camera
  init_input_system(r);

  move_to_scene_start(r, Scene::menu);
}

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
  if (!input.update_since_last_fixed_update) {
    // get last tick held inputs
    auto& last_tick_inputs = fixed_input.history[fixed_input.fixed_tick - 1];

    // append them to this tick
    auto& i = fixed_input.history[fixed_input.fixed_tick];

    // only append the held states
    const auto is_held = [](const InputEvent& e) { return e.state == InputState::held; };
    const auto [first, last] = std::ranges::remove_if(last_tick_inputs, is_held);
    i.insert(i.end(), first, last);
  }
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

  const auto& leveling_up = get_first_component<SINGLE_UILevelUpComponent>(game);
  if (leveling_up.show_menu)
    return; // dont progress game while leveling up

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
  //
  const glm::ivec2 mouse_pos = mouse_position_in_worldspace(r);

  {
    OPTICK_EVENT("(update)-game-tick");
    update_camera_system(r, dt, mouse_pos);
    update_audio_system(r);
    update_cursor_system(r, mouse_pos);

    // systems
    update_ux_hoverable(r);

    // effects
    update_scale_by_velocity_system(r, dt);
    update_screenshake_system(r, app.ms_since_launch / 1000.0f, dt);
    update_wiggle_up_and_down_system(r, dt);
    update_ux_hoverable_change_colour_system(r);
    update_particle_system(r, dt);
    update_sprite_spritestack_system(r, dt);

    const auto& state = get_first_component<SINGLETON_GameStateComponent>(r);
    auto& gameover = get_first_component<SINGLETON_GameOver>(r);

    const std::vector<Scene> valid_scenes{
      Scene::game,
      Scene::test_scene_gun,
    };
    const bool in_scene_of_interest = std::find(valid_scenes.begin(), valid_scenes.end(), scene.s) != valid_scenes.end();
    if (in_scene_of_interest && state.state == GameState::RUNNING && !gameover.game_is_over) {

      // Only keeping this here until I'm convinced that
      // putting all these systems in update isn't a mistake
      const uint64_t milliseconds_dt = dt * 1000.0f;

      //
      // animation
      update_actor_bodypart_head_system(r, dt, mouse_pos);
      update_actor_bodypart_legs_system(r, dt, mouse_pos);
      //
      // powerup
      update_combat_powerup_doubledamage_system(r, dt);
      //
      // ai
      update_set_velocity_to_target_system(r, dt);
      update_actor_group_system(r, mouse_pos);
      //
      // combat
      update_attack_cooldown_system(r, milliseconds_dt);
      update_take_damage_system(r);
      update_weapon_shotgun_system(r, milliseconds_dt);
      update_flash_sprite_system(r, milliseconds_dt);
      update_enemy_system(r, dt);
      update_wants_to_shoot_system(r);
      //
      // spawners
      update_spawner_system(r, milliseconds_dt);
      //
      // items
      update_intent_pickup_system(r);
      update_intent_drop_item_system(r);
      update_selected_interactions_system(r, mouse_pos, dt);
    }
  }

  {
    OPTICK_EVENT("(update)-update-render-system");
    update_animator_system(r, dt);
    update_render_system(r, dt, mouse_pos);
  }

  {
    OPTICK_EVENT("(update)-update-ui");
    // update_ui_inverse_kinematics_system(r, mouse_pos);

    // Main menu scene can transition to Game scene
    if (scene.s == Scene::menu) {
      update_ui_scene_main_menu(app, r);
      update_ui_rpg_character_system(r);
    }
    if (scene.s == Scene::game) {
      update_ui_inventory(r);
      update_ui_selected(r);
      update_ui_arrows_to_spawners_system(r);
      update_ui_player_name_above_player_system(r);
      update_ui_level_up_system(r);
      update_ui_xp_bar_system(r);
    }
    update_ui_worldspace_text_system(r);
    update_ui_pause_menu_system(app, r);
    update_ui_gameover_system(r);

    // todo: put in to a settings menu
    static bool show_settings_ui = false;
    if (show_settings_ui) {
      // update_ui_audio_system(r);
      update_ui_controller_system(r);
    }

#if defined(_DEBUG)
    // static bool show_demo_window = false;
    // ImGui::ShowDemoWindow(&show_demo_window);

    static bool show_editor_ui = true;
    if (show_editor_ui) {
      update_ui_hierarchy_system(r);
      update_ui_level_editor_system(r, mouse_pos);
      update_ui_collisions_system(r);
      update_ui_colour_palette_system(r);
    }

#endif
  }

  end_frame_render_system(r);
};
