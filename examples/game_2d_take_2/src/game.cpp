// your header
#include "game.hpp"

// game headers
#include "constants.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"

// systems
#include "systems/render_system.hpp"

// helpers

// engine headers
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

  Resources& r = registry.ctx<Resources>();
  // sprites
  std::string path_to_kennynl = "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl, path_to_kennynl);
  r.loaded_texture_ids = engine::load_textures_threaded(textures_to_load);

  { // create player entity
    entt::entity r = registry.create();
    registry.emplace<Position>(r, int(screen_wh.x / 2.0f), int(screen_wh.y / 2.0f));
    registry.emplace<Size>(r, 16, 16);
    registry.emplace<Colour>(r, 1.0f, 0.0f, 0.0f, 1.0f);
    registry.emplace<Sprite>(r, sprite::type::TREE_1);
  }
  {
    entt::entity r = registry.create();
    registry.emplace<Position>(r, int(screen_wh.x / 2.0f - 100), int(screen_wh.y / 2.0f));
    registry.emplace<Size>(r, 16, 16);
    registry.emplace<Colour>(r, 1.0f, 0.0f, 0.0f, 1.0f);
    registry.emplace<Sprite>(r, sprite::type::SQUARE);
  }
}

void
game2d::update(entt::registry& registry, float dt)
{
  Uint64 start_physics = SDL_GetPerformanceCounter();
  {
    // Physics
  }
  Uint64 end_physics = SDL_GetPerformanceCounter();
  float physics_elapsed_ms = (end_physics - start_physics) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  {
    // Input
  };

  {
    // Game Tick
  };

  Uint64 start_render = SDL_GetPerformanceCounter();
  {
    update_render_system(registry);
  };
  Uint64 end_render = SDL_GetPerformanceCounter();
  float render_elapsed_ms = (end_render - start_render) / float(SDL_GetPerformanceFrequency()) * 1000.0f;

  // UI
  {
    // bool show_imgui_demo_window = false;
    // ImGui::ShowDemoWindow(&show_imgui_demo_window);

    ImGui::Begin("Profiler");
    ImGui::Text("Physics %f", physics_elapsed_ms);
    ImGui::Text("Render %f", render_elapsed_ms);
    ImGui::End();
  };
}
