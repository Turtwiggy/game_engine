
// This example creates a window and shows the imgui demo window

// c standard library headers
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdio.h>

// other library headers
#include <glm/glm.hpp>
#include <imgui.h>

// your project headers
#include "engine/application.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/tools/profiler.hpp"
#include "engine/ui/profiler_panel.hpp"
#include "engine/util.hpp"
using namespace fightingengine;

int
main()
{
  std::cout << "(INFO) booting up..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool hide_windows_console = true;
  if (hide_windows_console)
    fightingengine::hide_windows_console();

  uint32_t width = 1366;
  uint32_t height = 768;
  glm::ivec2 screen_wh = { 1366, 768 };
  bool vsync = false;

  Application app("Hello Application", screen_wh.x, screen_wh.y, vsync);
  app.limit_fps = true;
  app.fps_if_limited = 60.0f;
  Profiler profiler;

  bool show_imgui_demo_window = true;

  log_time_since("(INFO) End Setup ", app_start);

  while (app.is_running()) {

    Uint64 frame_start_time = SDL_GetPerformanceCounter();
    profiler.new_frame();
    profiler.begin(Profiler::Stage::UpdateLoop);

    app.frame_begin(); // get input events
    float delta_time_s = app.get_delta_time();
    if (delta_time_s >= 0.25f)
      delta_time_s = 0.25f;

    profiler.begin(Profiler::Stage::SdlInput);
    {
      // Shutdown app
      if (app.get_input().get_key_down(SDL_SCANCODE_ESCAPE))
        app.shutdown();

      // Input examples
      if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_W))
        printf("w is being held! \n");
      else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_S))
        printf("s is being held! \n");
      if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_A))
        printf("a is being held! \n");
      else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_D))
        printf("d is being held! \n");
      if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_SPACE))
        printf("space is being held! \n");
      else if (app.get_input().get_key_held(SDL_Scancode::SDL_SCANCODE_LSHIFT))
        printf("left shift is being held! \n");
    }
    profiler.end(Profiler::Stage::SdlInput);
    profiler.begin(Profiler::Stage::GameTick);
    {
      // Some game tick stuff...!
    }
    profiler.end(Profiler::Stage::GameTick);
    profiler.begin(Profiler::Stage::Render);
    {
      // Example OpenGL usage
      Framebuffer::default_fbo();
      glm::vec4 dark_blue = glm::vec4(0.0f / 255.0f, 100.0f / 255.0f, 100.0f / 255.0f, 1.0f);
      RenderCommand::set_clear_colour(dark_blue);
      RenderCommand::clear();
    }
    profiler.end(Profiler::Stage::Render);
    profiler.begin(Profiler::Stage::GuiLoop);
    {
      // ImGui demo window
      ImGui::ShowDemoWindow(&show_imgui_demo_window);

      ImGui::Begin("Profiler");

      // Show FPS
      float framerate = ImGui::GetIO().Framerate;
      float framerate_ms = 1000.0f / ImGui::GetIO().Framerate;
      std::stringstream stream;
      stream << std::fixed << std::setprecision(2) << framerate;
      std::string framerate_str = stream.str();
      stream.str(std::string());
      stream << std::fixed << std::setprecision(2) << framerate;
      std::string framerate_ms_str = stream.str();
      std::string framerate_label = framerate_str + std::string(" FPS (") + framerate_ms_str + std::string(" ms)");
      ImGui::Text(framerate_label.c_str());

      profiler_panel::draw_timers(profiler, delta_time_s);
      ImGui::End();
    }
    profiler.end(Profiler::Stage::GuiLoop);
    profiler.begin(Profiler::Stage::FrameEnd);
    {
      app.frame_end(frame_start_time);
    }
    profiler.end(Profiler::Stage::FrameEnd);
    profiler.end(Profiler::Stage::UpdateLoop);
  }
  return 0;
}