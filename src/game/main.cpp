
// game headers
#include "game.hpp"
using namespace game2d;

#include "engine/app/application.hpp"
#include "engine/app/io.hpp"
using namespace engine;

// other libs
#include <entt/entt.hpp>
#include <imgui.h>

// std lib
#include <chrono>
#include <iostream>

// fixed tick
static const int MILLISECONDS_PER_FIXED_TICK = 7; // or ~142 ticks per second
// static int FIXED_MILLISECONDS_PER_TICK = 16; // or ~62.5 ticks per second
static uint64_t milliseconds_since_last_tick = 0;
static uint64_t now = 0;
static uint64_t last = 0;
static uint64_t milliseconds_delta_time = 0;

static entt::registry registry;

void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it
  auto& app = registry.ctx().at<SINGLETON_Application>();
  engine::start_frame(app);

  last = now;
  now = SDL_GetTicks64();
  milliseconds_delta_time = now - last;
  if (milliseconds_delta_time > 250)
    milliseconds_delta_time = 250; // avoid spiral

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  milliseconds_since_last_tick += milliseconds_delta_time;
  while (milliseconds_since_last_tick >= MILLISECONDS_PER_FIXED_TICK) {
    milliseconds_since_last_tick -= MILLISECONDS_PER_FIXED_TICK;

    game2d::fixed_update(registry, MILLISECONDS_PER_FIXED_TICK);
  }

  // Implement this if stuttering?
  // const double alpha = seconds_since_last_game_tick / SECONDS_PER_FIXED_TICK;
  // state = current_state * alpha + previous_state * (1.0f - alpha );

  game2d::update(registry, milliseconds_delta_time / 1000.0f);

  engine::end_frame(app);
}

int
main(int argc, char* argv[])
{
  IM_UNUSED(argc);
  IM_UNUSED(argv);

  { // init
    const auto start = std::chrono::high_resolution_clock::now();
    auto& app = registry.ctx().emplace<engine::SINGLETON_Application>();
    app.window = GameWindow("Fighting Game", app.width, app.height, app.display, app.vsync);
    app.imgui.initialize(app.window);
    log_time_since("(INFO) End init() ", start);
  }

  // bool hide_windows_console = false;
  // if (hide_windows_console)
  //   engine::hide_windows_console();

  game2d::init(registry);

  auto& app = registry.ctx().at<engine::SINGLETON_Application>();
  while (app.running)
    main_loop(nullptr);

  return 0;
}