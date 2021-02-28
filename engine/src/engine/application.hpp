#pragma once

// c++ standard library headers
#include <memory>
#include <string>

// your project headers
#include "engine/game_window.hpp"
#include "engine/imgui/imgui_setup.hpp"
#include "engine/input_manager.hpp"

namespace fightingengine {

class Application
{
public:
  Application(const std::string& name = "Fighting Engine (Default)", int width = 1080, int height = 720);
  ~Application();

  [[nodiscard]] bool is_running() const;
  void shutdown();

  [[nodiscard]] float get_delta_time();
  void frame_begin();
  void frame_end(const float delta_time);

  void gui_begin();
  void gui_end();

  void set_fps_limit(const float fps);
  void remove_fps_limit();

  [[nodiscard]] InputManager& get_input();
  [[nodiscard]] ImGui_Manager& get_imgui();
  [[nodiscard]] GameWindow& get_window();

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

  // FPS settings
  bool fps_limit = false;
  float FPS = 60.0;
  int MILLISECONDS_PER_FRAME = (int)(1000 / FPS);

  // Game's fixed tick
  // int FIXED_TICKS_PER_SECOND = 1;
  // float SECONDS_PER_FIXED_TICK = 1.0f / FIXED_TICKS_PER_SECOND;
  // float seconds_since_last_game_tick = 0;

  // Physics tick
  // int PHYSICS_TICKS_PER_SECOND = 60;
  // float SECONDS_PER_PHYSICS_TICK = 1.0f / PHYSICS_TICKS_PER_SECOND;

  // Delta time
  unsigned int start = 0;
  unsigned int prev = 0;
  unsigned int now = 0;
};
}
