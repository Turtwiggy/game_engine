
// game headers
#include "components/app.hpp"
#include "game.hpp"
using namespace game2d;

#include "app/application.hpp"
#include "app/io.hpp"
#include "opengl/util.hpp"
using namespace engine;

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

// other libs
#include <entt/entt.hpp>
#include <imgui.h>

// std lib
#include <chrono>

// fixed tick
static const int MILLISECONDS_PER_FIXED_TICK = 7; // or ~142 ticks per second
// static int MILLISECONDS_PER_FIXED_TICK = 16; // or ~62.5 ticks per second
static uint64_t milliseconds_accumulator_since_last_tick = 0;
static uint64_t new_time = 0;
static uint64_t cur_time = 0;
static uint64_t milliseconds_delta_time = 0;

static SINGLETON_Application app;
static GameEditor editor;
static Game game;

void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it

#if defined(__EMSCRIPTEN_PTHREADS__)
  std::cout << " Emscripten pthreads defined\n";
#endif

  engine::start_frame(app);

  new_time = SDL_GetTicks64();
  milliseconds_delta_time = new_time - cur_time;
  if (milliseconds_delta_time > 250)
    milliseconds_delta_time = 250; // avoid spiral
  cur_time = new_time;

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  milliseconds_accumulator_since_last_tick += milliseconds_delta_time;
  while (milliseconds_accumulator_since_last_tick >= MILLISECONDS_PER_FIXED_TICK) {
    milliseconds_accumulator_since_last_tick -= MILLISECONDS_PER_FIXED_TICK;

    game2d::fixed_update(editor, game, MILLISECONDS_PER_FIXED_TICK);
  }

  // Implement this if stuttering?
  // const float alpha = seconds_since_last_game_tick / SECONDS_PER_FIXED_TICK;
  // state = current_state * alpha + previous_state * (1.0f - alpha );

  game2d::update(app, editor, game, milliseconds_delta_time / 1000.0f);

  engine::end_frame(app);
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

  const auto start = std::chrono::high_resolution_clock::now();
  app.window = GameWindow("Dungeon", app.width, app.height, app.display, app.vsync);
  app.imgui.initialize(app.window);

#ifdef WIN32
  bool hide_windows_console = false;
  if (hide_windows_console)
    engine::hide_windows_console();
#endif

  game2d::init(app, editor, game);
  CHECK_OPENGL_ERROR(0);
  log_time_since("(INFO) End init()", start);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else
  while (app.running)
    main_loop(nullptr);
#endif

  return 0;
}