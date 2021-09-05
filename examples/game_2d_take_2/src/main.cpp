// game headers
#include "game.hpp"

#include "components/global_resources.hpp"
using namespace game2d;

// engine headers
#include "engine/application.hpp"
#include "engine/grid.hpp"
#include "engine/maths_core.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "engine/opengl_renderers/batch_renderers/sprite.hpp"
#include "engine/opengl_renderers/batch_renderers/triangle_fan.hpp"
#include "engine/util.hpp"
using namespace engine;

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

// std lib headers
#include <iostream>

// colour palette; https://colorhunt.co/palette/273312
constexpr glm::vec4 PALETTE_COLOUR_1_1 = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);  // black
constexpr glm::vec4 PALETTE_COLOUR_2_1 = glm::vec4(0.0f / 255.0f, 173.0f / 255.0f, 181.0f / 255.0f, 1.0f); // blue
constexpr glm::vec4 PALETTE_COLOUR_3_1 =
  glm::vec4(170.0f / 255.0f, 216.0f / 255.0f, 211.0f / 255.0f, 1.0f); // lightblue
constexpr glm::vec4 PALETTE_COLOUR_4_1 = glm::vec4(238.0f / 255.0f, 238.0f / 255.0f, 238.0f / 255.0f, 1.0f); // grey
constexpr glm::vec4 PALETTE_COLOUR_5_1 = glm::vec4(0.95f, 0.3f, 0.3f, 1.0f);
constexpr glm::vec4 background_colour = PALETTE_COLOUR_1_1; // black

int
main()
{
  std::cout << "init..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool use_vsync = true;
  bool show_imgui_demo_window = false;

  glm::ivec2 start_screen_wh = { 1366, 720 };
  Application app("2D Game", start_screen_wh.x, start_screen_wh.y, use_vsync);
  entt::registry registry;
  init(registry, start_screen_wh);

  log_time_since("(INFO) End Setup ", app_start);

  while (app.is_running()) {
    Uint64 frame_start_time = SDL_GetPerformanceCounter();

    app.frame_begin(); // input events
    float delta_time_s = app.get_delta_time();
    if (delta_time_s >= 0.25f)
      delta_time_s = 0.25f;

    // Physics

    // Input

    // Game Tick

    // Render
    Framebuffer::default_fbo();
    RenderCommand::set_clear_colour(background_colour);
    RenderCommand::clear();

    // UI

    // ImGui::ShowDemoWindow(&show_imgui_demo_window);

    app.frame_end(frame_start_time);
  }

  return 0;
}