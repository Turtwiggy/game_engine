#pragma once

#define IMGUI_IMPL_OPENGL_LOADER_GLEW

// your project headers
#include "engine/game_window.hpp"

namespace engine {

class ImGui_Manager
{
public:
  ~ImGui_Manager();

  void initialize(GameWindow* window);
  void begin_frame(const GameWindow& window);
  void end_frame(const GameWindow& window);
  void process_event(const SDL_Event* event);
};

} // namespace engine
