#pragma once

#include "engine/app/game_window.hpp"
#include "engine/app/setup_imgui.hpp"

namespace engine {

struct SINGLE_Application
{
  // defaults
  bool vsync = true;
  bool limit_fps = false;
  float fps_limit = 60.0f;

  GameWindow window;
  ImGui_Manager imgui;

  bool running = true;
  uint64_t frame_start_time = 0;
  uint64_t ms_since_launch = 0;
};

void
start_frame(SINGLE_Application& app);

void
end_frame(SINGLE_Application& app);

} // namespace engine