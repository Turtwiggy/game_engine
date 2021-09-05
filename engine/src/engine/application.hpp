#pragma once

// your project headers
#include "engine/game_window.hpp"

// engine headers
#include "engine/imgui/imgui_setup.hpp"
#include "engine/input_manager.hpp"

// c++ standard library headers
#include <memory>
#include <string>

namespace fightingengine {

class Application
{
public:
  Application(const std::string& name = "Fighting Engine (Default)",
              int width = 1080,
              int height = 720,
              bool vsync = true);
  ~Application();

  [[nodiscard]] bool is_running() const;
  void shutdown();

  [[nodiscard]] float get_delta_time();
  void frame_begin();
  void frame_end(Uint64& frame_start_time);

  float seconds_since_launch = 0.0f;

  float fps_if_limited = 60.0f;
  bool limit_fps = false;

  bool window_was_resized = false;

  [[nodiscard]] GameWindow& get_window();
  [[nodiscard]] InputManager& get_input();

private:
  // window events
  void on_window_close();
  void on_window_resize(int w, int h);

private:
  std::unique_ptr<GameWindow> window;
  InputManager input_manager;
  ImGui_Manager imgui_manager;

  bool running = true;
  bool minimized = false;

  // Game's fixed tick
  // int FIXED_TICKS_PER_SECOND = 1;
  // float SECONDS_PER_FIXED_TICK = 1.0f / FIXED_TICKS_PER_SECOND;
  // float seconds_since_last_game_tick = 0;

  // Physics tick
  // int PHYSICS_TICKS_PER_SECOND = 60;
  // float SECONDS_PER_PHYSICS_TICK = 1.0f / PHYSICS_TICKS_PER_SECOND;
};
}

// THIS IS FOR A FIXED GAME TICK
// -----------------------------
// seconds_since_last_game_tick += delta_time_in_seconds;
// while (seconds_since_last_game_tick >= SECONDS_PER_FIXED_TICK)
//{
//    //Fixed update
//    fixed_tick(SECONDS_PER_FIXED_TICK);
//    seconds_since_last_game_tick -= SECONDS_PER_FIXED_TICK;
//}
