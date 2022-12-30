#pragma once

#include "app/game_window.hpp"
#include "app/setup_imgui.hpp"

namespace engine {

struct SINGLETON_Application
{
  // defaults
  bool vsync = true;
  bool limit_fps = true;
  float fps_limit = 60.0f;
  int width = 1600;
  int height = 900;
  DisplayMode display = DisplayMode::windowed;
  GameWindow window;
  ImGui_Manager imgui;

  bool running = true;
  uint64_t frame_start_time = 0;
  uint64_t ms_since_launch = 0;
};

void
start_frame(SINGLETON_Application& app);

void
end_frame(SINGLETON_Application& app);

} // namespace engine