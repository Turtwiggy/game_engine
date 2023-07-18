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
#include "modules/player/system.hpp"
#include "modules/ui_main_menu/system.hpp"
#include "physics/components.hpp"
#include "renderer/components.hpp"
#include "renderer/system.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"
#include "ui_profiler/components.hpp"
#include "ui_profiler/helpers.hpp"

// temporary headers while wip stuff
#include "colour/colour.hpp"
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"

namespace game2d {

void
init_game(entt::registry& r)
{
  r.emplace<OrthographicCamera>(r.create());

  const auto player_type = EntityType::actor_player;
  const auto player = create_gameplay(r, player_type);
  create_renderable(r, player, player_type);
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

  r.emplace<Profiler>(r.create());
  r.emplace<SINGLETON_RendererInfo>(r.create());
  r.emplace<engine::RandomState>(r.create());
  r.emplace<SINGLETON_EntityBinComponent>(r.create());
  r.emplace<SINGLETON_FixedUpdateInputHistory>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());
  r.emplace<SINGLETON_PhysicsComponent>(r.create());
  r.emplace<GameStateComponent>(r.create());
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
game2d::fixed_update(entt::registry& game, uint64_t milliseconds_dt)
{
  auto& p = get_first_component<Profiler>(game);
  auto _ = time_scope(&p, "fixed_update()", true);

  auto& input = get_first_component<SINGLETON_InputComponent>(game);
  auto& fixed_input = get_first_component<SINGLETON_FixedUpdateInputHistory>(game);
  fixed_input.history.clear();

  // move inputs from Update() to this FixedUpdate() tick
  fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_inputs);
  const auto& inputs = fixed_input.history[fixed_input.fixed_tick];

  {
    // tick game logic

    // process inputs in FixedUpdateInputHistory
    update_player_controller_system(game, inputs, milliseconds_dt);

    // destroy objects
    auto& dead = get_first_component<SINGLETON_EntityBinComponent>(game);
    update_lifecycle_system(dead, game, milliseconds_dt);
  }

  fixed_input.fixed_tick += 1;
}

void
game2d::update(engine::SINGLETON_Application& app, entt::registry& r, float dt)
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
  static bool show_editor_ui = true;
  if (show_editor_ui) {
  }

  end_frame_render_system(r);
};
