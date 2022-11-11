// header
#include "engine/app/application.hpp"

void
engine::start_frame(SINGLETON_Application& app)
{
  app.frame_start_time = SDL_GetPerformanceCounter();
  app.imgui.begin_frame(app.window);
};

void
engine::end_frame(SINGLETON_Application& app)
{
  app.imgui.end_frame(app.window);
  SDL_GL_SwapWindow(app.window.get_handle());

  uint64_t frame_end_time = SDL_GetPerformanceCounter();
  float elapsed_ms =
    (frame_end_time - app.frame_start_time) / static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

  if (app.limit_fps) {
    auto delay = floor(1000.0f / app.fps_limit - elapsed_ms);
    if (delay > 0.0f)
      SDL_Delay(static_cast<Uint32>(delay));
  }
};