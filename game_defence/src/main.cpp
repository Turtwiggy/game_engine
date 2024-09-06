#include "app/application.hpp"
#include "app/io.hpp"
#include "game.hpp"
using namespace game2d;

#include "opengl/util.hpp"
using namespace engine;

#if defined(__EMSCRIPTEN__)
#include "deps/opengl.hpp"
#include <emscripten.h>
#endif

#if defined(_MSC_VER)
#include <optick.h>
#endif

// other libs
#include <SDL_timer.h>
#include <entt/entt.hpp>
#include <imgui.h>

// std lib
#include <fmt/core.h>
#include <optional>
#include <string>
#include <thread>

// fixed tick
static const int MILLISECONDS_PER_FIXED_TICK = 7; // or ~142 ticks per second
// static int MILLISECONDS_PER_FIXED_TICK = 16; // or ~62.5 ticks per second
static uint64_t cur_time = 0;
static uint64_t milliseconds_accumulator_since_last_tick = 0;

static SINGLETON_Application app;
static entt::registry game;

static int frames_to_pass_before_init = 3;
static bool done_init_slow = false;
static std::optional<std::thread> slow_thread = std::nullopt;

void
launch_thread_after_x_frames()
{
#if defined(__EMSCRIPTEN__)
  const bool do_threaded = false;
#else
  const bool do_threaded = true;
#endif

  if (do_threaded) {
    if (frames_to_pass_before_init <= 0 && !done_init_slow) {
      const auto work = []() { game2d::init_slow(app, game); };
      slow_thread = std::thread(work);
      done_init_slow = true;
      fmt::println("spawning thread...");
    }
    if (slow_thread != std::nullopt && slow_thread.value().joinable()) {
      fmt::println("want to join thread...");
      slow_thread.value().join();
      slow_thread = std::nullopt;
    }
  }

  if (!do_threaded) {
    // Just do the slow work. Non-threded.
    if (frames_to_pass_before_init <= 0 && !done_init_slow) {
      game2d::init_slow(app, game);
      done_init_slow = true;
    }
  }

  if (frames_to_pass_before_init > 0)
    frames_to_pass_before_init--;
}

void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it

#if defined(_MSC_VER)
  OPTICK_FRAME("MainThread");
#endif

  engine::start_frame(app);
  launch_thread_after_x_frames();

  const uint64_t new_time = SDL_GetTicks64();
  uint64_t frame_time = new_time - cur_time;
  if (frame_time > 250)
    frame_time = 250; // avoid spiral
  cur_time = new_time;

  milliseconds_accumulator_since_last_tick += frame_time;

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  while (milliseconds_accumulator_since_last_tick >= MILLISECONDS_PER_FIXED_TICK) {
    milliseconds_accumulator_since_last_tick -= MILLISECONDS_PER_FIXED_TICK;

    game2d::fixed_update(app, game, MILLISECONDS_PER_FIXED_TICK);
  }

  const float dt = frame_time / 1000.0f;
  game2d::update(app, game, dt);

  engine::end_frame(app);
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

#if (defined(WIN32) || defined(_WIN32))
  fmt::println("Hello, Windows!");
  bool hide_windows_console = true;
  if (hide_windows_console) {
    fmt::println("hiding console...");
    engine::hide_windows_console();
  }
#endif

#if defined(__EMSCRIPTEN__)
  fmt::println("Hello, Emscripten!");
#endif

#if defined(__EMSCRIPTEN_PTHREADS__)
  fmt::println("Emscripten pthreads defined");
#endif

#if defined(__APPLE__)
  fmt::println("Hello, Apple!");
#endif

#if defined(__unix__)
  fmt::println("Hello, Unix!");
#endif

  // const auto start = std::chrono::high_resolution_clock::now();

  std::string name = "SOLAR WARFARE";

#if defined(_DEBUG)
  name += " [DEBUG]";
#else
  name += " [RELEASE]";
#endif
#if defined(__MINGW32__)
  name += "[MINGW]";
#endif
#if defined(_MSC_VER)
  name += "[MSVC]";
#endif

  // #if defined(_DEBUG)
  //   app.limit_fps = true;
  //   app.fps_limit = 30;
  // #endif

  app.window = GameWindow(name, DisplayMode::windowed, app.vsync);
  app.imgui.initialize(app.window);

  game2d::init(app, game);
  CHECK_OPENGL_ERROR(0);

#if defined(__EMSCRIPTEN__)
  fmt::println("about to start main loop...");
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else

#if defined(_MSC_VER)
  OPTICK_START_CAPTURE();
#endif

  while (app.running)
    main_loop(nullptr);

#if defined(_MSC_VER)
  OPTICK_STOP_CAPTURE();
#endif
  // OPTICK_SAVE_CAPTURE("GameCapture");
#endif

  return 0;
}