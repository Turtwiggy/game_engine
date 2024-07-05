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
#include <optick.h>

// std lib
#include <fmt/core.h>
#include <thread>

// fixed tick
static const int MILLISECONDS_PER_FIXED_TICK = 7; // or ~142 ticks per second
// static int MILLISECONDS_PER_FIXED_TICK = 16; // or ~62.5 ticks per second
static uint64_t milliseconds_accumulator_since_last_tick = 0;
static uint64_t new_time = 0;
static uint64_t cur_time = 0;
static uint64_t milliseconds_delta_time = 0;
static const int allowed_ticks_per_frame = 2;

static SINGLETON_Application app;
static entt::registry game;

static int frames_to_pass_before_init = 2;
static bool done_init_slow = false;
static std::optional<std::thread> slow_thread = std::nullopt;

void
launch_thread_after_x_frames()
{
  if (frames_to_pass_before_init <= 0 && !done_init_slow) {
    const auto work = []() { game2d::init_slow(app, game); };
    slow_thread = std::thread(work);
    done_init_slow = true;
  }
  if (slow_thread.has_value() && slow_thread.value().joinable()) {
    slow_thread.value().join();
    slow_thread = std::nullopt;
  }
  if (frames_to_pass_before_init > 0)
    frames_to_pass_before_init--;
}

void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it
  OPTICK_FRAME("MainThread");

#if defined(__EMSCRIPTEN_PTHREADS__)
  fmt::println("Emscripten pthreads defined");
#endif

  engine::start_frame(app);
  launch_thread_after_x_frames();

  new_time = SDL_GetTicks64();
  milliseconds_delta_time = new_time - cur_time;
  if (milliseconds_delta_time > MILLISECONDS_PER_FIXED_TICK * allowed_ticks_per_frame)
    milliseconds_delta_time = MILLISECONDS_PER_FIXED_TICK * allowed_ticks_per_frame; // avoid spiral
  cur_time = new_time;

  milliseconds_accumulator_since_last_tick += milliseconds_delta_time;

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  while (milliseconds_accumulator_since_last_tick >= MILLISECONDS_PER_FIXED_TICK) {
    milliseconds_accumulator_since_last_tick -= MILLISECONDS_PER_FIXED_TICK;

#if defined(_DEBUG)
    const auto before_fixed_update = SDL_GetTicks64();
#endif

    game2d::fixed_update(app, game, MILLISECONDS_PER_FIXED_TICK);

#if defined(_DEBUG)
    const auto after_fixed_update = SDL_GetTicks64();
    const auto execution_time = after_fixed_update - before_fixed_update;
    if (execution_time > MILLISECONDS_PER_FIXED_TICK)
      fmt::println("uh oh! in trouble! fixedupdate() is taking too long");
#endif
  }

  game2d::update(app, game, milliseconds_delta_time / 1000.0f);

  engine::end_frame(app);
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

#if defined(WIN32) || defined(_WIN32)
  fmt::println("Hello, Windows!");
  bool hide_windows_console = true;
  if (hide_windows_console)
    engine::hide_windows_console();
#endif

#if defined(__unix__)
  fmt::println("Hello, Unix!");
#endif

  // const auto start = std::chrono::high_resolution_clock::now();

  std::string name = "Solar War";
#if defined(_DEBUG)
  name += " [DEBUG]";
#else
  name += " [RELEASE]";
#endif

  app.height = 720;
  app.width = 1280;
  app.window = GameWindow(name, app.width, app.height, app.display, app.vsync);
  app.imgui.initialize(app.window);

  game2d::init(app, game);
  CHECK_OPENGL_ERROR(0);

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else
  // OPTICK_START_CAPTURE();

  while (app.running)
    main_loop(nullptr);

    // OPTICK_STOP_CAPTURE();
    // OPTICK_SAVE_CAPTURE("GameCapture");
#endif

  return 0;
}