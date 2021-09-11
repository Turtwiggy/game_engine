// your header
#include "game.hpp"

// game headers
#include "constants.hpp"

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
  init_ui_system(registry);

  Resources& r = registry.ctx<Resources>();
  r.rnd = engine::RandomState();
  auto& rng = r.rnd.rng;

  // sprites
  std::string path_to_kennynl = "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl, path_to_kennynl);
  r.loaded_texture_ids = engine::load_textures_threaded(textures_to_load);

  int x_offset = 32;
  int y_offset = 64;

  for (int i = 0; i < 40; i++) {
    for (int j = 0; j < 20; j++) {
      entt::entity r = registry.create();

      int pos_x = int(x_offset + i * grid_size);
      int pos_y = int(y_offset + j * grid_size);

      float rnd = engine::rand_det_s(rng, 0.3f, 0.6f);
      Colour base;
      base.colour = { rnd, rnd, rnd, 1.0f };

      registry.emplace<Colour>(r, base);
      registry.emplace<Position>(r, pos_x, pos_y);
      registry.emplace<Size>(r, grid_size, grid_size);
      registry.emplace<Sprite>(r, sprite::type::SQUARE);
      registry.emplace<ZIndex>(r, 0);
    }
  }

  { // add a player
    entt::entity r = registry.create();

    Colour base;
    base.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
    Hoverable h;
    h.hover_colour.colour = { 0.0f, 1.0f, 0.0f, 1.0f };

    registry.emplace<Colour>(r, base);
    registry.emplace<Hoverable>(r, h);
    registry.emplace<Player>(r);
    registry.emplace<Position>(r, 100, 100);
    registry.emplace<Size>(r, grid_size, grid_size);
    registry.emplace<Sprite>(r, sprite::type::PERSON_1);
    registry.emplace<ZIndex>(r, 1);
  }
}

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
  {
    //
  };

  // game tick
  {
    update_hover_system(registry, app);
    update_player_system(registry, app);
  };

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
}
