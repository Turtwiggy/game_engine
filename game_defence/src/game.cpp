#include "game.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "maths/maths.hpp"
#include "modules/actor_cursor/system.hpp"
#include "modules/actor_dropoff_zone_requets_items/system.hpp"
#include "modules/actor_enemy/system.hpp"
#include "modules/actor_player/system.hpp"
#include "modules/actor_spawner/components.hpp"
#include "modules/actor_spawner/system.hpp"
#include "modules/actor_turret/system.hpp"
#include "modules/actor_weapon_bow/system.hpp"
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
#include "modules/gameover/components.hpp"
#include "modules/gameover/system.hpp"
#include "modules/items_drop/system.hpp"
#include "modules/items_pickup/system.hpp"
#include "modules/lerp_to_target/components.hpp"
#include "modules/lerp_to_target/system.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/lifecycle/system.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/respawn/system.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/screenshake/system.hpp"
#include "modules/selected_interactions/system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_collisions/system.hpp"
#include "modules/ui_colours/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_dropoff_zone/system.hpp"
#include "modules/ui_gameover/system.hpp"
#include "modules/ui_grid_interaction/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_inventory/system.hpp"
#include "modules/ui_inverse_kinematics/system.hpp"
#include "modules/ui_level_editor/components.hpp"
#include "modules/ui_level_editor/system.hpp"
#include "modules/ui_next_wave/system.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "modules/ui_selected/system.hpp"
#include "modules/ui_spawner_editor/system.hpp"
#include "modules/ux_hoverable/system.hpp"
#include "physics/components.hpp"
#include "physics/process_actor_actor_collisions.hpp"
#include "physics/process_move_objects.hpp"
#include "resources/colours.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "imgui.h"
#include "optick.h"

#include <ranges>
#include <vector>

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

    Texture kennynl_particle;
    kennynl_particle.path = "assets/textures/kenny_particle_pack/kenny_muzzle_spritesheet.png";
    kennynl_particle.spritesheet_path = "assets/config/spritemap_kennynl_particle.json";
    ri.user_textures.push_back(kennynl_particle);

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
    update_camera_system(r, dt);
    update_audio_system(r);
    update_cursor_system(r, mouse_pos);

    const auto& state = get_first_component<SINGLETON_GameStateComponent>(r);
    const auto& gameover = get_first_component<SINGLETON_GameOver>(r);
    if (scene.s == Scene::game && state.state == GameState::RUNNING && !gameover.game_is_over) {

      // Only keeping this here until I'm convinced that
      // putting all these systems in update isn't a mistake
      const uint64_t milliseconds_dt = dt * 1000.0f;

      // update_flash_sprite_system(game, milliseconds_dt);
      // update_turret_system(game, milliseconds_dt);
      //
      // ai
      update_set_velocity_to_target_system(r, dt);
      //
      // combat
      update_attack_cooldown_system(r, milliseconds_dt);
      update_take_damage_system(r);
      update_enemy_system(r);
      update_wants_to_shoot_system(r);
      //
      // spawners
      update_respawn_system(r);
      update_spawner_system(r, milliseconds_dt);
      //
      // items
      update_intent_pickup_system(r);
      update_intent_drop_item_system(r);
      update_actor_dropoffzone_request_items(r, milliseconds_dt);
      //
      // effects
      update_scale_by_velocity_system(r, dt);
      update_screenshake_system(r, app.ms_since_launch / 1000.0f, dt);
      update_wiggle_up_and_down_system(r, dt);
      //
      // systems using inputs or inputs from fixedupate

      // problem:
      // update() generates a ton of inputs
      // fixedupdate() collates those, and decides on an action e.g. shoot
      // two fixedupdates() could occur back-to-back,
      // so update() could never recieve the input.shoot event

      // solution:
      // fixedupdate() collates inputs, then adds an e.g.
      // WantsToShoot event
      // this is then processed in the next update()

      // Does this mean networked-clients operate differently?

      update_bow_system(r, milliseconds_dt);
      update_weapon_shotgun_system(r, milliseconds_dt);
      update_ux_hoverable(r);
      update_selected_interactions_system(r, mouse_pos);
    }
  }

  {
    OPTICK_EVENT("(update)-update-render-system");
    update_animator_system(r, dt);
    update_render_system(r, dt);
  }

  {
    OPTICK_EVENT("(update)-update-ui");
    // update_ui_inverse_kinematics_system(r, mouse_pos);

    if (scene.s == Scene::menu) {
      update_ui_scene_main_menu(app, r);
    } else if (scene.s == Scene::game) {
      update_ui_grid_interaction_system(r);
      update_ui_inventory(r);
      update_ui_dropoff_zone_system(r);
      update_ui_selected(r);
    }

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
      auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
      // update_ui_colours_system(colours);
    }

#endif
  }

  end_frame_render_system(r);
};
