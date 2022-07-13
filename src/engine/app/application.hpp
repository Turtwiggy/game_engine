#pragma once

// your project headers
#include "engine/app/game_window.hpp"

// engine headers
#include "engine/app/setup_imgui.hpp"

// c++ standard library headers
#include <memory>
#include <string>

namespace engine {

class Application
{
public:
  Application(const std::string& name = "Fighting Engine (Default)",
              int width = 1080,
              int height = 720,
              bool vsync = true);
  ~Application();

  bool is_running();
  void shutdown();
  [[nodiscard]] float get_delta_time();
  [[nodiscard]] double get_seconds_since_launch();
  [[nodiscard]] int get_frame();

  void frame_begin();
  void frame_end(Uint64& frame_start_time);

  float fps_if_limited = 60.0f;
  bool limit_fps = false;

  ImGui_Manager imgui_manager;
  std::unique_ptr<GameWindow> window;

private:
  bool running = true;
  bool minimized = false;
};
}