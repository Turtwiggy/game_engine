// your header
#include "game.hpp"

// components
#include "components/profiler.hpp"
#include "components/resources.hpp"

// systems
#include "systems/hex_grid.hpp"
#include "systems/move_objects.hpp"
#include "systems/player_input.hpp"
#include "systems/render.hpp"
#include "systems/ui.hpp"

// game2d headers
#include "factories.hpp"

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
  Resources& res = registry.ctx<Resources>();
  glm::ivec2 screen_center = { screen_wh.x / 2, screen_wh.y / 2 };

  // sprites
  const std::string path_to_kennynl = "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl, path_to_kennynl);
  res.loaded_texture_ids = engine::load_textures_threaded(textures_to_load);

  // Add a player
  auto player_0 = create_player(registry, screen_center);

  init_ui_system(registry);
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
    update_move_objects_system(registry, app);
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
    update_ui_system(registry, app);
  };

  // end of frame
  {
    end_frame_render_system(registry);
  }
};
