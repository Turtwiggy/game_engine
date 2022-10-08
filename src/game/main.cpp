
// game headers
#include "game.hpp"
#include "game/components/app.hpp"
using namespace game2d;

#include "engine/app/application.hpp"
#include "engine/app/io.hpp"
using namespace engine;

// other libs
#include <entt/entt.hpp>
#include <imgui.h>

// std lib
#include <chrono>

// fixed tick
static const int MILLISECONDS_PER_FIXED_TICK = 7; // or ~142 ticks per second
// static int MILLISECONDS_PER_FIXED_TICK = 16; // or ~62.5 ticks per second
static uint64_t milliseconds_since_last_tick = 0;
static uint64_t now = 0;
static uint64_t last = 0;
static uint64_t milliseconds_delta_time = 0;

static SINGLETON_Application app;
static GameEditor editor;
static Game game;

void
main_loop(void* arg)
{
  IM_UNUSED(arg); // do nothing with it

  engine::start_frame(app);

  last = now;
  now = SDL_GetTicks64();
  milliseconds_delta_time = now - last;

  // const int max_frames_to_process = 2;
  // const int cap_ms = (MILLISECONDS_PER_FIXED_TICK * max_frames_to_process) + 1;
  if (milliseconds_delta_time > 250)
    milliseconds_delta_time = 250; // avoid spiral

  // The physics cycle may happen more than once per frame if
  // the fixed timestep is less than the actual frame update time.
  milliseconds_since_last_tick += milliseconds_delta_time;
  while (milliseconds_since_last_tick >= MILLISECONDS_PER_FIXED_TICK) {
    milliseconds_since_last_tick -= MILLISECONDS_PER_FIXED_TICK;

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
  app.window = GameWindow("Fighting Game", app.width, app.height, app.display, app.vsync);
  app.imgui.initialize(app.window);

  // bool hide_windows_console = false;
  // if (hide_windows_console)
  //   engine::hide_windows_console();

  game2d::init(app, editor, game);
  log_time_since("(INFO) End init()", start);

  while (app.running)
    main_loop(nullptr);

  return 0;
}