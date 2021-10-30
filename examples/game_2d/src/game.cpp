// your header
#include "game.hpp"

// components
#include "components/parry.hpp"
#include "components/player.hpp"
#include "components/singleton_grid.hpp"
#include "components/singleton_resources.hpp"
#include "components/velocity_in_boundingbox.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ui_profiler/components.hpp"

// helpers
#include "helpers/physics_layers.hpp"

// systems
#include "modules/physics/system.hpp"
#include "modules/renderer/system.hpp"
#include "modules/ui_hierarchy/system.hpp"
#include "modules/ui_profiler/system.hpp"
#include "systems/move_objects.hpp"
#include "systems/parry.hpp"
#include "systems/player_input.hpp"
#include "systems/velocity_in_boundingbox.hpp"

// engine headers
#include "engine/maths.hpp"
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
    registry.emplace<TagComponent>(r, "player0");
    registry.emplace<Player>(r);
    registry.emplace<VelocityComponent>(r);
    registry.emplace<ColourComponent>(r, 1.0f, 1.0f, 1.0f, 1.0f);
    registry.emplace<PositionIntComponent>(r, 30 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::PERSON_1);
    CollisionLayer layer(static_cast<int>(GameCollisionLayer::PLAYER));
    registry.emplace<CollidableComponent>(r, layer);
  }

  // Add a cursor
  {
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "cursor");
    registry.emplace<ColourComponent>(r, 1.0f, 0.0f, 0.0f, 0.5f);
    registry.emplace<PositionIntComponent>(r);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::EMPTY);
    registry.emplace<CursorComponent>(r);
  }

  // Add ball object
  {
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "ball");
    registry.emplace<VelocityInBoundingboxComponent>(r);
    registry.emplace<VelocityComponent>(r, -200.0f, 200.0f);
    registry.emplace<ColourComponent>(r, 1.0f, 1.0f, 1.0f, 1.0f);
    registry.emplace<PositionIntComponent>(r, 50 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::EMPTY);
    registry.emplace<ParryComponent>(r);
    CollisionLayer layer(static_cast<int>(GameCollisionLayer::OTHER));
    registry.emplace<CollidableComponent>(r, layer);
  }

  // Add goal object
  {
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "goal");
    registry.emplace<ColourComponent>(r, 0.0f, 1.0f, 0.0f, 1.0f);
    registry.emplace<PositionIntComponent>(r, 10 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::EMPTY);
    CollisionLayer layer(static_cast<int>(GameCollisionLayer::GOAL));
    registry.emplace<CollidableComponent>(r, layer);
  }

  init_physics_system(registry);
  init_ui_profiler_system(registry);
  init_ui_hierarchy_system(registry);
};

void
game2d::update(entt::registry& registry, engine::Application& app, float dt)
{
  Profiler& p = registry.ctx<Profiler>();

  // physics
  Uint64 start_physics = SDL_GetPerformanceCounter();
  {
    update_physics_system(registry, app, dt);
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
    update_velocity_in_boundingbox_system(registry, app, dt);
    update_parry_system(registry, app, dt);
  };
  Uint64 end_game_tick = SDL_GetPerformanceCounter();
  p.game_tick_elapsed_ms = (end_game_tick - start_game_tick) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // rendering
  Uint64 start_render = SDL_GetPerformanceCounter();
  {
    update_render_system(registry, app);
  };
  Uint64 end_render = SDL_GetPerformanceCounter();
  p.render_elapsed_ms = (end_render - start_render) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // ui
  {
    update_ui_profiler_system(registry, app);
    update_ui_hierarchy_system(registry, app);
  };

  // end of frame
  {
    end_frame_render_system(registry);
  }
};
