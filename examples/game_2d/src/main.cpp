// game headers
#include "game.hpp"
using namespace game2d;

// engine headers
#include "engine/application.hpp"
#include "engine/util.hpp"
using namespace engine;

// other lib headers
#include <entt/entt.hpp>

// std lib headers
#include <iostream>

int
main()
{
  bool use_vsync = true;

  std::cout << "init..." << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  bool hide_windows_console = false;
  if (hide_windows_console)
    engine::hide_windows_console();

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

    game2d::update(registry, app, delta_time_s);

    app.frame_end(frame_start_time);
  }

  return 0;
}