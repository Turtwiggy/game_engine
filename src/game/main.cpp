
// game headers
#include "game.hpp"
using namespace game2d;

// engine headers
#include "engine/app/application.hpp"
#include "engine/app/io.hpp"
using namespace engine;

// other libs
#include <entt/entt.hpp>
#include <imgui.h>

// std lib
#include <chrono>
#include <iostream>

static bool vsync = true;
static bool limit_fps = false;
static float fps_limit = 60.0f;
static glm::ivec2 start_screen_wh = { 600, 600 * 9 / 16 };
static Application app("2D Game [0.0.8]", start_screen_wh.x, start_screen_wh.y, vsync);
static entt::registry registry;

// fixed tick
static int FIXED_TICKS_PER_SECOND = 60;
static float SECONDS_PER_FIXED_TICK = 1.0f / FIXED_TICKS_PER_SECOND;
static float seconds_since_last_game_tick = 0.0f;

void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it

  Uint64 frame_start_time = SDL_GetPerformanceCounter();

  app.frame_begin();

  float delta_time_s = app.get_delta_time();
  if (delta_time_s > 0.25f)
    delta_time_s = 0.25f;

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  seconds_since_last_game_tick += delta_time_s;
  while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK) {
    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;

    game2d::fixed_update(registry, app, SECONDS_PER_FIXED_TICK);
  }

  // Implement this if stuttering?
  // const double alpha = seconds_since_last_game_tick / SECONDS_PER_FIXED_TICK;
  // state = current_state * alpha + previous_state * (1.0f - alpha );

  game2d::update(registry, app, delta_time_s);

  app.frame_end(frame_start_time);
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

  std::cout << "Running main()" << std::endl;
  const auto app_start = std::chrono::high_resolution_clock::now();

  // configure application
  app.limit_fps = limit_fps;
  app.fps_if_limited = fps_limit;

  game2d::init(registry, start_screen_wh);

  log_time_since("(INFO) End Setup ", app_start);

#if defined(__EMSCRIPTEN__)
  printf("Hello, Emscripten!");

  // This function call won't return, and will engage in an infinite loop,
  // processing events from the browser, and dispatching them.
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else

  bool hide_windows_console = false;
  if (hide_windows_console)
    engine::hide_windows_console();

  while (app.is_running())
    main_loop(nullptr);
#endif

  return 0;
}