
// header
#include "engine/app/application.hpp"

// other lib headers
#include <imgui.h>

namespace engine {

Application::Application(const std::string& name, int width, int height, bool vsync)
{
  // const std::string kBuildStr(kGitSHA1Hash, 8);
  std::string kBuildStr = name;
#ifdef _DEBUG
  kBuildStr = kBuildStr + std::string(" [DEBUG]");
#endif

  window = std::make_unique<GameWindow>(kBuildStr, width, height, DisplayMode::WINDOWED, vsync);

  imgui_manager.initialize(window.get());

  running = true;
}

Application::~Application()
{
  window->close();
}

bool
Application::is_running()
{
  return running;
}

void
Application::shutdown()
{
  running = false;
}

float
Application::get_delta_time()
{
  return ImGui::GetIO().DeltaTime;
}

double
Application::get_seconds_since_launch()
{
  return ImGui::GetTime();
}

int
Application::get_frame()
{
  return ImGui::GetFrameCount();
}

void
Application::frame_begin()
{
  imgui_manager.begin_frame(*window);
}

void
Application::frame_end(Uint64& frame_start_time)
{
  imgui_manager.end_frame(*window);

  SDL_GL_SwapWindow(window->get_handle());

  Uint64 frame_end_time = SDL_GetPerformanceCounter();
  float elapsed_ms = (frame_end_time - frame_start_time) / static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

  if (limit_fps) {
    auto delay = floor(1000.0f / fps_if_limited - elapsed_ms);
    if (delay > 0.0f)
      SDL_Delay(static_cast<Uint32>(delay));
  }
}

}
