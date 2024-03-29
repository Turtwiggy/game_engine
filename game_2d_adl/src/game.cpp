#include "game.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/system.hpp"
#include "game_state.hpp"
#include "lifecycle/components.hpp"
#include "lifecycle/system.hpp"
#include "maths/maths.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/camera/system.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_pause_menu/system.hpp"
#include "modules/ui_scene_main_menu/system.hpp"
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
    SINGLETON_RendererInfo ri;

    Texture kennynl;
    kennynl.path = "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
    kennynl.spritesheet_path = "assets/config/spritemap_kennynl.json";
    ri.user_textures.push_back(kennynl);

    // load spritesheet info
    SINGLE_Animations anims;
    for (const auto& tex : ri.user_textures)
      load_sprites(anims, tex.spritesheet_path);
    r.emplace<SINGLE_Animations>(r.create(), anims);

    r.emplace<SINGLETON_RendererInfo>(r.create(), ri);
  }

  r.emplace<SINGLETON_FixedUpdateInputHistory>(r.create());
  r.emplace<SINGLETON_InputComponent>(r.create());

  const auto camera = r.create();
  r.emplace<TagComponent>(camera, "camera");
  OrthographicCamera camera_info;
  camera_info.projection = calculate_ortho_projection(app.width, app.height);
  r.emplace<OrthographicCamera>(camera, camera_info);
  r.emplace<TransformComponent>(camera);

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  init_render_system(app, r, ri); // init after camera
  init_input_system(r);

  move_to_scene_start(r, Scene::menu);
}

void
game2d::fixed_update(entt::registry& game, const uint64_t milliseconds_dt)
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
    const auto is_held = [](const InputEvent& e) { return e.state == InputState::held; };
    const auto [first, last] = std::ranges::remove_if(last_tick_inputs, is_held);

    // append them to this tick
    auto& i = fixed_input.history[fixed_input.fixed_tick];
    i.insert(i.end(), first, last);
    // std::cout << "multiple fixed tick in a row; appending held inputs" << std::endl;
  }
  input.update_since_last_fixed_update = false;

  // move unprocessed inputs from Update() to this FixedUpdate() tick
  const auto inputs = std::move(fixed_input.history[fixed_input.fixed_tick]);
  fixed_input.history.clear();
  fixed_input.history[fixed_input.fixed_tick] = std::move(inputs);

  auto& state = get_first_component<SINGLETON_GameStateComponent>(game);
  if (state.state == GameState::PAUSED)
    return; // note: this ignores inputs

  // destroy/create objects
  update_lifecycle_system(game, milliseconds_dt);

  {
    OPTICK_EVENT("(physics-tick)");
    auto& physics = get_first_component<SINGLETON_PhysicsComponent>(game);
    physics.frame_collisions.clear();

    update_move_objects_system(game, milliseconds_dt);
    update_actor_actor_collisions_system(game, physics);
  }

  {
    OPTICK_EVENT("fixed-game-tick");
    //...
  }

  fixed_input.fixed_tick += 1;
}

void
game2d::update(engine::SINGLETON_Application& app, entt::registry& r, const float dt)
{
  OPTICK_EVENT("(update)");
  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);

  // one frame behind
  const glm::ivec2 mouse_pos = mouse_position_in_worldspace(r);

  {
    OPTICK_EVENT("(update)-game-tick");
    update_input_system(app, r);
    update_camera_system(r, dt);
  }

  {
    OPTICK_EVENT("(update)-update-render-system");
    update_render_system(r, dt);
  }

  {
    OPTICK_EVENT("(update)-update-ui");

    if (scene.s == Scene::menu)
      update_ui_scene_main_menu(app, r);
    else if (scene.s == Scene::game) {
      //
    }
    update_ui_pause_menu_system(app, r);
  }

  end_frame_render_system(r);
};
