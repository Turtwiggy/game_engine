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
#include "modules/camera/orthographic.hpp"
#include "modules/camera/system.hpp"
#include "modules/enemy/system.hpp"
#include "modules/player/system.hpp"
#include "modules/spawner/components.hpp"
#include "modules/spawner/system.hpp"
#include "modules/turret/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_main_menu/system.hpp"
#include "modules/ui_prefabs/system.hpp"
#include "physics/components.hpp"
#include "physics/process_actor_actor.hpp"
#include "physics/process_move_objects.hpp"
#include "renderer/components.hpp"
#include "renderer/system.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"
#include "ui_profiler/components.hpp"
#include "ui_profiler/helpers.hpp"
#include "ui_profiler/system.hpp"

namespace game2d {

void
init_game(entt::registry& r)
{
  const auto camera = create_gameplay(r, EntityType::camera);

  const auto player = create_gameplay(r, EntityType::actor_player);
  auto& player_pos = r.get<TransformComponent>(player);
  player_pos.position.x = 50;
  player_pos.position.y = 50;

  const auto spawner = create_gameplay(r, EntityType::spawner);
  auto& spawner_pos = r.get<TransformComponent>(spawner);
  spawner_pos.position.x = 200;
  spawner_pos.position.y = 200;
}

} // namespace game2d

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
    // audio.sounds.push_back({ "MENU", "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" });
    // audio.sounds.push_back({ "HIT", "assets/audio/usfx_1_4/VOICES/Martial_Arts_Male/VOICE_Martial_Art_Shout_03_mono.wav"
    // }); audio.sounds.push_back({ "ATTACKED", "assets/audio/usfx_1_4/WEAPONS/Melee/Hammer/HAMMER_Hit_Body_stereo.wav" });
    r.emplace<SINGLETON_AudioComponent>(r.create(), audio);
  }

  r.emplace<engine::RandomState>(r.create());
  r.emplace<Profiler>(r.create());
  r.emplace<SINGLETON_RendererInfo>(r.create());
  r.emplace<SINGLETON_EntityBinComponent>(r.create());
  r.emplace<SINGLETON_FixedUpdateInputHistory>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  r.emplace<SINGLETON_PhysicsComponent>(r.create());
  r.emplace<SINGLETON_GameStateComponent>(r.create());
  r.emplace<SINGLETON_ColoursComponent>(r.create());

  auto& textures = get_first_component<SINGLETON_Textures>(r).textures;
  auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  auto& input = get_first_component<SINGLETON_InputComponent>(r);
  init_audio_system(audio);
  init_render_system(app, ri, textures);
  init_input_system(input);

  init_game(r);
}

void
game2d::fixed_update(entt::registry& game, const uint64_t milliseconds_dt)
{
  auto& p = get_first_component<Profiler>(game);
  auto _ = time_scope(&p, "fixed_update()", true);

  auto& input = get_first_component<SINGLETON_InputComponent>(game);
  auto& fixed_input = get_first_component<SINGLETON_FixedUpdateInputHistory>(game);
  fixed_input.history.clear();

  // move inputs from Update() to this FixedUpdate() tick
  fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);
  const auto& inputs = fixed_input.history[fixed_input.fixed_tick];
  fixed_input.fixed_tick += 1;

  // dont tick game logic if paused
  auto& state = get_first_component<SINGLETON_GameStateComponent>(game);
  if (state.state == GameState::PAUSED)
    return; // note: this ignores inputs

  // destroy/create objects
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(game);
  update_lifecycle_system(dead, game, milliseconds_dt);

  // update physics/collisions
  {
    auto _ = time_scope(&p, "(physics)", true);
    auto& physics = get_first_component<SINGLETON_PhysicsComponent>(game);
    physics.frame_collisions.clear();
    // move actors,
    // generate actor-solid collisions
    update_move_objects_system(game, milliseconds_dt);
    // generate actor-actor collisions
    update_actor_actor_system(game, physics);

    const auto resolve_collisions = [&game, &physics]() {
      auto& r = game;
      auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);
      for (const auto& coll : physics.collision_stay) {
        const auto a = static_cast<entt::entity>(coll.ent_id_0);
        const auto b = static_cast<entt::entity>(coll.ent_id_1);
        const auto a_type = r.get<EntityTypeComponent>(a).type;
        const auto b_type = r.get<EntityTypeComponent>(b).type;

        // bullet-enemy collision
        if (a_type == EntityType::actor_enemy && b_type == EntityType::actor_bullet) {
          dead.dead.emplace(a);
          dead.dead.emplace(b);
        } else if (a_type == EntityType::actor_bullet && b_type == EntityType::actor_enemy) {
          dead.dead.emplace(b);
          dead.dead.emplace(a);
        }

        // Hack: fix solid-solid enemy collision
        if (a_type == EntityType::actor_enemy && b_type == EntityType::actor_enemy) {
          dead.dead.emplace(a);
          dead.dead.emplace(b);
        }
      }
    };
    resolve_collisions();
  }

  // update gamelogic
  {
    auto _ = time_scope(&p, "(game-tick)", true);
    update_player_controller_system(game, inputs, milliseconds_dt);
    update_enemy_system(game, milliseconds_dt);
    update_turret_system(game, milliseconds_dt);
    update_spawner_system(game, milliseconds_dt);
  }
}

void
game2d::update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  auto& p = get_first_component<Profiler>(r);
  auto _ = time_scope(&p, "update()");

  {
    auto _ = time_scope(&p, "game_tick");
    auto& input = get_first_component<SINGLETON_InputComponent>(r);
    auto& audio = get_first_component<SINGLETON_AudioComponent>(r);
    update_input_system(app, input);
    update_camera_system(r, dt);
    update_audio_system(audio);
  };

  {
    auto _ = time_scope(&p, "rendering");
    auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    auto& texs = get_first_component<SINGLETON_Textures>(r).textures;
    auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
    update_render_system(ri, *colours.lin_background, *colours.background, texs, r, p);
  }

  // UI
  update_ui_main_menu_system(app, r);
  update_ui_prefabs_system(r);
  update_ui_hierarchy_system(r);
  static bool show_editor_ui = true;
  if (show_editor_ui) {
    auto& profiler = get_first_component<Profiler>(r);
    auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
    update_ui_profiler_system(profiler, physics, r);
  }

  end_frame_render_system(r);
};
