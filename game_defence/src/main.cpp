#include "game.hpp"
using namespace game2d;

#include "app/application.hpp"
#include "app/io.hpp"
#include "box2d/box2d.h"
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
static entt::registry game;
static b2Vec2 gravity = { 0.0f, 0.0f };
static b2World world{ gravity };

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

    game2d::fixed_update(game, world, MILLISECONDS_PER_FIXED_TICK);
  }

  game2d::update(app, game, world, milliseconds_delta_time / 1000.0f);

  engine::end_frame(app);
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

  const auto start = std::chrono::high_resolution_clock::now();

  std::string name = "Space";
#if defined(_DEBUG)
  name += " [DEBUG]";
#else
  name += " [RELEASE]";
#endif

  app.window = GameWindow(name, app.width, app.height, app.display, app.vsync);
  app.imgui.initialize(app.window);

  // #if defined(WIN32) && !defined(_DEBUG)
  //   bool hide_windows_console = false;
  //   if (hide_windows_console)
  //     engine::hide_windows_console();
  // #endif

  game2d::init(app, world, game);
  CHECK_OPENGL_ERROR(0);

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(main_loop, NULL, 0, true);
#else
  while (app.running)
    main_loop(nullptr);
#endif

  return 0;
}