// your header
#include "game.hpp"

// systems&components&helpers
#include "modules/audio/system.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/keyboard.hpp"
#include "modules/events/system.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/networking/system.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/system.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/system.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_networking/components.hpp"
#include "modules/ui_networking/system.hpp"
#include "modules/ui_physics/system.hpp"
#include "modules/ui_profiler/components.hpp"
#include "modules/ui_profiler/helpers.hpp"
#include "modules/ui_profiler/system.hpp"

#include "engine/app/application.hpp"
#include "game/entities/actors.hpp"
#include "game/simulate.hpp"
#include "game/systems/player.hpp"
#include "game/systems/player_inputs.hpp"
#include "game/systems/resolve_collisions.hpp"
#include "modules/camera/components.hpp"
#include "resources/audio.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

// other lib
#include <glm/glm.hpp>

namespace game2d {

void
init_game_state(entt::registry& r)
{
  r.each([&r](auto entity) { r.destroy(entity); });
  r.ctx().at<SINGLETON_PhysicsComponent>() = SINGLETON_PhysicsComponent();
  r.ctx().at<SINGLETON_GameOverComponent>() = SINGLETON_GameOverComponent();
  r.ctx().at<SINGLETON_HierarchyComponent>() = SINGLETON_HierarchyComponent();
  r.ctx().at<SINGLETON_EntityBinComponent>() = SINGLETON_EntityBinComponent();
  r.ctx().at<SINGLETON_AsteroidGameStateComponent>() = SINGLETON_AsteroidGameStateComponent();
  r.ctx().at<SINGLETON_FixedUpdateInputHistory>() = SINGLETON_FixedUpdateInputHistory();
  // init_shop_system(r);

  auto& gs = r.ctx().at<SINGLETON_AsteroidGameStateComponent>();
  create_hierarchy_root_node(r);

  auto player = create_player(r);
  auto& player_transform = r.get<TransformComponent>(player);
  player_transform.position.x = 200;
  player_transform.position.y = 200;
  auto& player_speed = r.get<PlayerComponent>(player);
  player_speed.speed = 250;
};

} // namespace game2d

void
game2d::init(entt::registry& r)
{
  const auto& app = r.ctx().at<engine::SINGLETON_Application>();

  // init once only
  r.ctx().emplace<Profiler>();
  r.ctx().emplace<SINGLETON_Textures>();
  r.ctx().emplace<SINGLETON_ColoursComponent>();
  init_sprite_system(r);
  init_render_system(r, { app.width, app.height });
  init_input_system(r);
  init_audio_system(r);
  init_networking_system(r);

  // emplace game things once (as they are delete/reinit at any time)
  r.ctx().emplace<SINGLETON_PhysicsComponent>();
  r.ctx().emplace<SINGLETON_GameOverComponent>();
  r.ctx().emplace<SINGLETON_HierarchyComponent>();
  r.ctx().emplace<SINGLETON_EntityBinComponent>();
  r.ctx().emplace<SINGLETON_AsteroidGameStateComponent>();
  r.ctx().emplace<SINGLETON_FixedUpdateInputHistory>();

  init_game_state(r);
};

void
game2d::fixed_update(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& p = r.ctx().at<Profiler>();

  {
    auto _ = time_scope(&p, "physics", true);
    auto& input = r.ctx().at<SINGLETON_InputComponent>();
    auto& fixed_input = r.ctx().at<SINGLETON_FixedUpdateInputHistory>();

    // while offline, just clear out anything older than a tick (until a sys needs older input)
    fixed_input.history.clear();

    // move inputs from Update() to this FixedUpdate() tick
    fixed_input.history[fixed_input.fixed_tick] = std::move(input.unprocessed_update_inputs);
    auto& inputs = fixed_input.history[fixed_input.fixed_tick];

    simulate(r, inputs, milliseconds_dt);
    // update_turret_system(r);
    // update_asteroid_system(r);

    fixed_input.fixed_tick += 1;

    // update_networking_system(r, milliseconds_dt);
  }
}

void
game2d::update(entt::registry& r, float dt)
{
  auto& p = r.ctx().at<Profiler>();
  const auto& ri = r.ctx().at<SINGLETON_RendererInfo>();
  auto& app = r.ctx().at<engine::SINGLETON_Application>();

  {
    auto _ = time_scope(&p, "game_tick");
    update_input_system(r);
    const auto& input = r.ctx().at<SINGLETON_InputComponent>();
    if (ri.viewport_process_events) {
      if (get_key_down(input, SDL_SCANCODE_R))
        init_game_state(r);
      if (get_key_down(input, SDL_SCANCODE_F))
        app.window.toggle_fullscreen();
      if (get_key_down(input, SDL_SCANCODE_ESCAPE))
        app.running = false;
    }

    // ... systems that always update
    {
      // update_cursor_system(r);
      update_audio_system(r);
    }

    // ... systems that update only if viewport is focused or hovered
    {
      if (ri.viewport_process_events) {
        // update_camera_system(r);
        update_player_inputs_system(r);
      }
    }
  };

  {
    auto _ = time_scope(&p, "rendering");
    update_sprite_system(r, dt);
    update_render_system(r); // put rendering on thread?
  };

  // ui
  {
    // TODO: fix this
    bool is_release = false;
    if (!is_release) {
      auto _ = time_scope(&p, "ui"); // value always be a frame behind
      update_ui_physics_system(r);
      update_ui_hierarchy_system(r);
      update_ui_profiler_system(r);
      // update_ui_shop_system(r);
      // update_ui_player_system(r);
      // update_ui_place_entity_system(r);
    }
    // update_ui_networking_system(r);
    // update_ui_main_menu_system(r);
  };

  // end frame
  {
    end_frame_render_system(r);
  }
};
