// your header
#include "game.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/hoverable.hpp"
#include "components/player.hpp"
#include "components/position.hpp"
#include "components/profiler_stats.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"
#include "components/z_index.hpp"

// systems
#include "systems/hex_grid_system.hpp"
#include "systems/hover_system.hpp"
#include "systems/player_system.hpp"
#include "systems/render_system.hpp"
#include "systems/ui_system.hpp"

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

  // sprites
  std::string path_to_kennynl = "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl, path_to_kennynl);
  res.loaded_texture_ids = engine::load_textures_threaded(textures_to_load);

  init_ui_system(registry);
  init_hex_grid_system(registry);
}

void
game2d::update(entt::registry& registry, engine::Application& app, float dt)
{
  ProfilerStats& p = registry.ctx<ProfilerStats>();

  // physics
  // Uint64 start_physics = SDL_GetPerformanceCounter();
  // {
  //   //
  // }
  // Uint64 end_physics = SDL_GetPerformanceCounter();
  // p.physics_elapsed_ms = (end_physics - start_physics) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // input
  Uint64 start_input = SDL_GetPerformanceCounter();
  {
    update_hover_system(registry, app);

#ifdef _DEBUG
    if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE)) {
      app.shutdown();
    }
#endif
  };
  Uint64 end_input = SDL_GetPerformanceCounter();
  p.input_elapsed_ms = (end_input - start_input) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // game tick
  Uint64 start_game_tick = SDL_GetPerformanceCounter();
  {
    update_hex_grid_system(registry, app, dt);
    update_player_system(registry, app);
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

  // end of frame things
  {
    end_frame_render_system(registry);
  }
}
