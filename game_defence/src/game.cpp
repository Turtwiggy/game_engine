#include "game.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "audio/system.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "lifecycle/system.hpp"
#include "maths/maths.hpp"
#include "modules/animation/scale_by_velocity.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/camera/system.hpp"
#include "modules/combat/components.hpp"
#include "modules/combat/flash_sprite.hpp"
#include "modules/combat/take_damage.hpp"
#include "modules/enemy/system.hpp"
#include "modules/gameover/components.hpp"
#include "modules/gameover/system.hpp"
#include "modules/items/intent_drop_item.hpp"
#include "modules/items/intent_pickup_item.hpp"
#include "modules/physics/components.hpp"
#include "modules/physics/process_actor_actor_collisions.hpp"
#include "modules/physics/process_move_objects.hpp"
#include "modules/player/system.hpp"
#include "modules/resolve_collisions/system.hpp"
#include "modules/respawn/system.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/spawner/components.hpp"
#include "modules/spawner/system.hpp"
#include "modules/turret/system.hpp"
#include "modules/ui_audio/system.hpp"
#include "modules/ui_controllers/system.hpp"
#include "modules/ui_economy/components.hpp"
#include "modules/ui_economy/system.hpp"
#include "modules/ui_gameover/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_next_wave/system.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_prefabs/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
#include "renderer/components.hpp"
#include "renderer/system.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"
#include "ui_profiler/components.hpp"
#include "ui_profiler/helpers.hpp"
#include "ui_profiler/system.hpp"

void
game2d::init(engine::SINGLETON_Application& app, entt::registry& r)
{
  {
    SINGLETON_Animations anims;
    SINGLETON_Textures textures;
    {
      Texture kenny_texture;
      kenny_texture.path = std::string("assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");
      kenny_texture.spritesheet_path = std::string("assets/config/spritemap_kennynl.json");
      load_sprites(anims.animations, kenny_texture.spritesheet_path);
      textures.textures.push_back(kenny_texture);
    }
    init_textures(textures);
    r.emplace<SINGLETON_Textures>(r.create(), textures);
    r.emplace<SINGLETON_Animations>(r.create(), anims);
  }

  {
    SINGLETON_AudioComponent audio;
    audio.sounds.push_back({ "SHOOT_01", "assets/audio/FIREARM_RTS_Machine_Gun_Model_01_Fire_Single_RR1_mono.wav" });
    audio.sounds.push_back({ "SHOOT_02", "assets/audio/FIREARM_Handgun_B_FS92_9mm_Fire_RR1_stereo.wav" });
    r.emplace<SINGLETON_AudioComponent>(r.create(), audio);
  }

  r.emplace<SINGLETON_Profiler>(r.create());
  r.emplace<SINGLETON_RendererInfo>(r.create());
  r.emplace<SINGLETON_FixedUpdateInputHistory>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());

  auto& textures = get_first_component<SINGLETON_Textures>(r).textures;
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  init_audio_system(r);
  init_render_system(app, ri, textures);
  init_input_system(r);

  move_to_scene_start(r, Scene::menu);
}

void
game2d::fixed_update(entt::registry& game, const uint64_t milliseconds_dt)
{
  auto& p = get_first_component<SINGLETON_Profiler>(game);
  auto _ = time_scope(&p, "fixed_update()", true);

  auto& input = get_first_component<SINGLETON_InputComponent>(game);
  auto& fixed_input = get_first_component<SINGLETON_FixedUpdateInputHistory>(game);
  fixed_input.history.clear();

  // move inputs from Update() to this FixedUpdate() tick
  fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);
  const auto& inputs = fixed_input.history[fixed_input.fixed_tick];

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
    auto _ = time_scope(&p, "(physics)-tick", true);
    auto& p = get_first_component<SINGLETON_Profiler>(game);
    auto& physics = get_first_component<SINGLETON_PhysicsComponent>(game);
    physics.frame_collisions.clear();

    {
      auto _ = time_scope(&p, "(physics)-tick-move", true);
      update_move_objects_system(game, milliseconds_dt);
    }
    {
      auto _ = time_scope(&p, "(physics)-actor-actor-colls", true);
      update_actor_actor_collisions_system(game, physics);
    }
    {
      auto _ = time_scope(&p, "(physics)-resolve-collisions", true);
      update_resolve_collisions_system(game);
    }
  }

  {
    auto& p = get_first_component<SINGLETON_Profiler>(game);
    auto _ = time_scope(&p, "(fixed-tick)-game-logic", true);
    update_player_controller_system(game, milliseconds_dt);
    update_enemy_system(game, milliseconds_dt);
    update_turret_system(game, milliseconds_dt);
    update_take_damage_system(game);
    update_flash_sprite_system(game, milliseconds_dt);
    update_respawn_system(game);
    update_spawner_system(game, milliseconds_dt);
    update_intent_pickup_system(game);
    update_intent_drop_item_system(game);
  }

  fixed_input.fixed_tick += 1;
}

void
game2d::update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  auto& p = get_first_component<SINGLETON_Profiler>(r);
  auto _ = time_scope(&p, "update()");

  {
    auto _ = time_scope(&p, "update()-tick");
    auto& input = get_first_component<SINGLETON_InputComponent>(r);
    auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
    update_input_system(app, input);
    update_camera_system(r, dt);
    update_audio_system(r);
    update_scale_by_velocity_system(r, dt);
  };

  {
    auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    auto& texs = get_first_component<SINGLETON_Textures>(r).textures;
    update_render_system(r, texs);
  }

  // static bool show = true;
  // ImGui::ShowDemoWindow(&show);

  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  if (scene.s == Scene::menu)
    update_ui_scene_main_menu(app, r);
  else if (scene.s == Scene::game) {
    update_ui_next_wave_system(r);
  }
  update_ui_pause_menu_system(app, r);
  update_ui_gameover_system(r);

  static bool show_editor_ui = true;
  if (show_editor_ui) {
    update_ui_profiler_system(r);
    update_ui_prefabs_system(r);
    update_ui_controller_system(r);
    update_ui_economy_system(r);
    update_ui_audio_system(r);
    update_ui_hierarchy_system(r);
  }

  end_frame_render_system(r);
};
