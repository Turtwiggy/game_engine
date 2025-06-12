#pragma once

#if defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_ES3
#endif

// your project headers
#include "engine/app/game_window.hpp"

namespace engine {

class ImGui_Manager
{
public:
  ~ImGui_Manager();

  void initialize(GameWindow& window);
  void begin_frame();
  void end_frame(const GameWindow& window);
  void process_event(const SDL_Event* event);
};

} // namespace engine
