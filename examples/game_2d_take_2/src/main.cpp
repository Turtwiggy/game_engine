
// game headers

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
using namespace fightingengine;

// other lib headers
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

// std lib headers
#include <iostream>

int
main()
{
  std::cout << "init..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  RandomState rnd;

  // config
  bool use_vsync = false;
  bool show_imgui_demo_window = false;

  glm::ivec2 screen_wh = { 1366, 720 };
  Application app("2D Game", screen_wh.x, screen_wh.y, use_vsync);
  app.limit_fps = true;
  app.fps_if_limited = 120.0f;

  // std::vector<std::pair<int, std::string>> textures_to_load;
  // textures_to_load.emplace_back(tex_unit_kenny_nl,
  //                               "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");

  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(screen_wh.x), static_cast<uint32_t>(screen_wh.y));
  RenderCommand::set_depth_testing(false); // disable depth testing for 2d
  // sprite_renderer::init();
  // triangle_fan_renderer::init();
  // print_gpu_info();
  sprite_renderer::SpriteBatchRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  entt::registry registry;

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
    glm::vec4 dark_blue = glm::vec4(0.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
    RenderCommand::set_clear_colour(dark_blue);
    RenderCommand::clear();

    // UI

    // ImGui demo window
    ImGui::ShowDemoWindow(&show_imgui_demo_window);

    app.frame_end(frame_start_time);
  }

  return 0;
}