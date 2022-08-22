#pragma once

#include "engine/app/game_window.hpp"
#include "engine/app/setup_imgui.hpp"

namespace engine {

struct SINGLETON_Application
{
  // defaults
  bool vsync = true;
  bool limit_fps = true;
  float fps_limit = 60.0f;
  int width = 1400;
  int height = width * 9 / 16.0f;
  DisplayMode display = DisplayMode::WINDOWED;
  GameWindow window;
  ImGui_Manager imgui;

  bool running = true;
  uint64_t frame_start_time = 0;
};

void
start_frame(SINGLETON_Application& app);

void
end_frame(SINGLETON_Application& app);

} // namespace engine