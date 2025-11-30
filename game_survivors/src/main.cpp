#include "pch.hpp"

#include "game.hpp"
using namespace game2d;

#include "engine/app/application.hpp"
#include "engine/app/io.hpp"
#include "engine/opengl/util.hpp"
using namespace engine;

#if defined(__EMSCRIPTEN__)
#include "engine/deps/opengl.hpp"
#include <emscripten.h>
#endif

#if defined(_DEBUG) && !defined(TRACY_ENABLE)
#define TRACY_ENABLE
#endif
#if defined(_DEBUG)
#define TRACY_CALLSTACKS 32
#include <tracy/Tracy.hpp>
#endif

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/actors/actor_player/components.hpp"

// fixed tick
// static constexpr int MILLISECONDS_PER_FIXED_TICK = 7; // or ~142 ticks per second
static constexpr int MILLISECONDS_PER_FIXED_TICK = 16; // or ~62.5 ticks per second
static uint64_t cur_time = 0;
static uint64_t milliseconds_accumulator_since_last_tick = 0;

static SINGLE_Application app;
static entt::registry game;
static std::optional<std::thread> slow_thread = std::nullopt;

void
launch_thread()
{
#if defined(__EMSCRIPTEN__)
  const bool do_threaded = false;
#else
  const bool do_threaded = true;
#endif

  if (do_threaded) {
    const auto work = []() { game2d::init_slow(app, game); };
    slow_thread = std::thread(work);
    SDL_Log("%s", "spawning thread...");
    slow_thread.value().join();
    SDL_Log("%s", "joined thread...");
  }

  // Just do the slow work. Non-threaded.
  if (!do_threaded)
    game2d::init_slow(app, game);
}

void
main_loop(void* arg)
{
#if defined(_DEBUG)
  ZoneScopedS(32);
#endif
  IM_UNUSED(arg); // do nothing with it

  engine::start_frame(app);

  const uint64_t now = SDL_GetTicks64();
  app.start_ms = now;
  app.frame_s = SDL_GetPerformanceCounter();

  uint64_t frame_time = now - cur_time;
  if (frame_time > 250)
    frame_time = 250; // avoid spiral
  cur_time = now;

  milliseconds_accumulator_since_last_tick += frame_time;

  // float speed = 1.0f;
  // const auto& input_c = get_first_component<SINGLE_InputComponent>(game);
  // if (get_key_held(input_c, SDL_SCANCODE_PERIOD))
  //   speed = 5.0f;
  // const auto g_input_e = get_first<InputComponent, Persistent>(game);
  // const auto& g_input_c = game.get<InputComponent>(g_input_e);
  // const auto& b = g_input_c.ability2;
  // const bool do_act = std::find(b.begin(), b.end(), ActionStateEnum::HELD) != b.end();
  // if (do_act)
  //   speed = 5.0f;

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  while (milliseconds_accumulator_since_last_tick >= MILLISECONDS_PER_FIXED_TICK) {
    milliseconds_accumulator_since_last_tick -= MILLISECONDS_PER_FIXED_TICK;

    game2d::fixed_update(app, game, MILLISECONDS_PER_FIXED_TICK);
  }

  game2d::update(app, game, frame_time);

  engine::end_frame(app);

#if defined(_DEBUG)
  FrameMark; // frame done
#endif
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

#if (defined(WIN32) || defined(_WIN32))
  SDL_Log("%s", "Hello, Windows!");
  bool hide_windows_console = true;
  if (hide_windows_console) {
    SDL_Log("%s", "hiding console...");
    engine::hide_windows_console();
  }
#endif

#if defined(__EMSCRIPTEN__)
  SDL_Log("%s", "Hello, Emscripten!");
#endif

#if defined(__EMSCRIPTEN_PTHREADS__)
  SDL_Log("%s", "Emscripten pthreads defined");
#endif

#if defined(__APPLE__)
  SDL_Log("%s", "Hello, Apple!");
#endif

#if defined(__unix__)
  SDL_Log("%s", "Hello, Unix!");
#endif

  // const auto start = std::chrono::high_resolution_clock::now();

  std::string name = "Oh Buoy!";

#if defined(_DEBUG)
  name += " [DEBUG]";
#else
  name += " [RELEASE]";
#endif
#if defined(__MINGW32__)
  name += "[g++]";
#endif
#if defined(_MSC_VER)
  name += "[MSVC]";
#endif

  app.vsync = true;
  // app.limit_fps = false;
  // app.fps_limit = 120;

#if defined(_DEBUG)
  app.window = GameWindow(name, DisplayMode::windowed_borderless, app.vsync);
#else
  app.window = GameWindow(name, DisplayMode::fullscreen_borderless, app.vsync);
#endif

  app.imgui.initialize(app.window);

  launch_thread();
  game2d::init(app, game);
  CHECK_OPENGL_ERROR(0);

#if defined(__EMSCRIPTEN__)
  SDL_Log("%s", "about to start main loop...");
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else

  while (app.running)
    main_loop(nullptr);

#endif

  SteamAPI_Shutdown();

  return 0;
}