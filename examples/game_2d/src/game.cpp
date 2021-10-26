// your header
#include "game.hpp"

// components
#include "components/grid_square.hpp"
#include "components/parry.hpp"
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/profiler.hpp"
#include "components/rendering.hpp"
#include "components/resources.hpp"

// systems
#include "systems/move_objects.hpp"
#include "systems/player_input.hpp"
#include "systems/prototype/clamp_to_screen.hpp"
#include "systems/prototype/parry.hpp"
#include "systems/render.hpp"
#include "systems/ui_profiler.hpp"

// engine headers
#include "engine/maths_core.hpp"
#include "engine/util.hpp"

// other project headers
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <imgui.h>

// c++ lib headers
#include <string>
#include <utility>
#include <vector>

void
game2d::init(entt::registry& registry, glm::ivec2 screen_wh)
{
  init_render_system(registry, screen_wh);

  registry.set<SINGLETON_ResourceComponent>(SINGLETON_ResourceComponent());
  registry.set<SINGLETON_GridSize>(SINGLETON_GridSize());

  // access singleton data
  const auto& res = registry.ctx<SINGLETON_ResourceComponent>();
  const auto& gs = registry.ctx<SINGLETON_GridSize>();
  const int GRID_SIZE = gs.size_xy;

  // Add a player
  {
    entt::entity r = registry.create();
    registry.emplace<Player>(r);
    registry.emplace<Velocity>(r);
    registry.emplace<Colour>(r, 1.0f, 1.0f, 1.0f, 1.0f);
    registry.emplace<PositionInt>(r, 30 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<Size>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<Sprite>(r, sprite::type::PERSON_1);
    registry.emplace<ZIndex>(r, 0);
  }

  // Add a cursor
  {
    entt::entity r = registry.create();
    registry.emplace<Colour>(r, 1.0f, 0.0f, 0.0f, 0.5f);
    registry.emplace<PositionInt>(r);
    registry.emplace<Size>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<Sprite>(r, sprite::type::EMPTY);
    registry.emplace<ZIndex>(r, 1);
    registry.emplace<PlayerCursor>(r);
  }

  // Add a default ball aiming at player
  {
    entt::entity r = registry.create();
    registry.emplace<Velocity>(r, -100.0f, 0.0f);
    registry.emplace<Colour>(r, 1.0f, 1.0f, 1.0f, 1.0f);
    registry.emplace<PositionInt>(r, 50 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<Size>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<Sprite>(r, sprite::type::EMPTY);
    registry.emplace<ZIndex>(r, 0);
    registry.emplace<Bouncy>(r);
  }

  init_ui_profiler_system(registry);
};

void
game2d::update(entt::registry& registry, engine::Application& app, float dt)
{
  ProfilerStats& p = registry.ctx<ProfilerStats>();

  // physics
  Uint64 start_physics = SDL_GetPerformanceCounter();
  {
    //
  }
  Uint64 end_physics = SDL_GetPerformanceCounter();
  p.physics_elapsed_ms = (end_physics - start_physics) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // input
  Uint64 start_input = SDL_GetPerformanceCounter();
  {
    update_player_input_system(registry, app);

#ifdef _DEBUG
    if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE)) {
      app.shutdown();
    }
#endif
  };
  Uint64 end_input = SDL_GetPerformanceCounter();
  p.input_elapsed_ms = (end_input - start_input) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // game logic
  Uint64 start_game_tick = SDL_GetPerformanceCounter();
  {
    update_move_objects_system(registry, app, dt);
    update_clamp_to_screen_system(registry, app, dt);
    update_parry_system(registry, app, dt);
  };
  Uint64 end_game_tick = SDL_GetPerformanceCounter();
  p.game_tick_elapsed_ms = (end_game_tick - start_game_tick) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // rendering
  Uint64 start_render = SDL_GetPerformanceCounter();
  {
    update_render_system(registry);
  };
  Uint64 end_render = SDL_GetPerformanceCounter();
  p.render_elapsed_ms = (end_render - start_render) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // ui
  {
    update_ui_profiler_system(registry, app);
  };

  // end of frame
  {
    end_frame_render_system(registry);
  }
};
