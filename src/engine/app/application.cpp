// header
#include "engine/app/application.hpp"

#include <SDL_timer.h>

#if defined(_DEBUG)
#include <tracy/Tracy.hpp>
#endif

void
engine::start_frame(SINGLE_Application& app)
{
  app.imgui.begin_frame(app.window);
};

void
engine::end_frame(SINGLE_Application& app)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  app.imgui.end_frame(app.window);
  SDL_GL_SwapWindow(app.window.get_handle());

  if (app.limit_fps) {
    const uint64_t end_s = SDL_GetPerformanceCounter();
    const float elapsed_ms = (end_s - app.frame_s) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
    const float target_ms = (1000.0f / app.fps_limit);
    const float delay_ms = floor(target_ms - elapsed_ms);
    if (delay_ms > 0.0f)
      SDL_Delay(static_cast<Uint32>(delay_ms));
  }
};