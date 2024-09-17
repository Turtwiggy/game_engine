// header
#include "engine/app/application.hpp"

void
engine::start_frame(SINGLE_Application& app)
{
  app.frame_start_time = SDL_GetPerformanceCounter();
  app.imgui.begin_frame(app.window);
};

void
engine::end_frame(SINGLE_Application& app)
{
  app.imgui.end_frame(app.window);
  SDL_GL_SwapWindow(app.window.get_handle());

  app.ms_since_launch = SDL_GetTicks64();

  if (app.limit_fps) {
    const uint64_t frame_end_time = SDL_GetPerformanceCounter();
    const float elapsed_ms = (frame_end_time - app.frame_start_time) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    const float target_ms = (1000.0f / app.fps_limit);
    const float delay_ms = floor(target_ms - elapsed_ms);
    if (delay_ms > 0.0f)
      SDL_Delay(static_cast<Uint32>(delay_ms));
  }
};