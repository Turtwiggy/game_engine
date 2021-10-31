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
#include "systems/destroy_on_collide.hpp"
#include "systems/move_objects.hpp"
#include "systems/parry.hpp"
#include "systems/player_input.hpp"
#include "systems/process_physics.hpp"
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
  init_physics_system(registry);
  init_ui_profiler_system(registry);
  init_ui_hierarchy_system(registry);

  registry.set<SINGLETON_ResourceComponent>(SINGLETON_ResourceComponent());
  registry.set<SINGLETON_GridSize>(SINGLETON_GridSize());

  // colours
  const glm::vec4 colour_red = glm::vec4(232 / 255.0f, 80 / 255.0f, 100 / 255.0f, 1.0f);
  const glm::vec4 colour_cyan = glm::vec4(8 / 255.0f, 177 / 255.0f, 190 / 255.0f, 1.0f);
  const glm::vec4 colour_dblue = glm::vec4(49 / 255.0f, 99 / 255.0f, 188 / 255.0f, 1.0f);
  const glm::vec4 colour_white = glm::vec4(1.0f);

  // access singleton data
  const auto& res = registry.ctx<SINGLETON_ResourceComponent>();
  const auto& gs = registry.ctx<SINGLETON_GridSize>();
  const int GRID_SIZE = gs.size_xy;

  // Add a player
  {
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "player");
    registry.emplace<Player>(r);
    registry.emplace<VelocityComponent>(r);
    registry.emplace<ColourComponent>(r, colour_cyan);
    registry.emplace<PositionIntComponent>(r, 25 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::PERSON_1);
    registry.emplace<CollidableComponent>(r, static_cast<uint32_t>(GameCollisionLayer::PLAYER));
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

  // Add goal object
  {
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "goal");
    registry.emplace<ColourComponent>(r, colour_white);
    registry.emplace<PositionIntComponent>(r, 10 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::EMPTY);
    registry.emplace<CollidableComponent>(r, static_cast<uint32_t>(GameCollisionLayer::GOAL));
  }

  // Add wall object
  {
    entt::entity r = registry.create();
    registry.emplace<TagComponent>(r, "wall");
    registry.emplace<ColourComponent>(r, colour_red);
    registry.emplace<PositionIntComponent>(r, 16 * GRID_SIZE, 25 * GRID_SIZE);
    registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
    registry.emplace<SpriteComponent>(r, sprite::type::EMPTY);
    registry.emplace<CollidableComponent>(r, static_cast<uint32_t>(GameCollisionLayer::WALL));
  }

  // Add balls
  {
    for (int i = 1; i < 2; i++) {
      entt::entity r = registry.create();
      registry.emplace<TagComponent>(r, std::string("ball" + std::to_string(i)));
      registry.emplace<VelocityInBoundingboxComponent>(r);
      registry.emplace<VelocityComponent>(r, -5.0f, 0.0f);
      registry.emplace<ColourComponent>(r, colour_dblue);
      registry.emplace<PositionIntComponent>(r, (30) * GRID_SIZE, 25 * GRID_SIZE);
      registry.emplace<SizeComponent>(r, GRID_SIZE, GRID_SIZE);
      registry.emplace<SpriteComponent>(r, sprite::type::EMPTY);
      registry.emplace<ParryComponent>(r);
      registry.emplace<CollidableComponent>(r, static_cast<uint32_t>(GameCollisionLayer::BALL));
    }
  }
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
    update_process_physics_system(registry, app, dt);
    // update_destroy_on_collide_system(registry, app, dt);
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
