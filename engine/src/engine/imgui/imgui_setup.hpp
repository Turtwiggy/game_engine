#pragma once

#define IMGUI_IMPL_OPENGL_LOADER_GLEW

// c++ standard library headers
#include <array>
#include <memory>

// other library headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <imgui.h>

// your project headers
#include "engine/core/game_window.hpp"

namespace fightingengine {

class ImGui_Manager
{
public:
  ~ImGui_Manager();

  void initialize(GameWindow* window);

  void begin_frame(const GameWindow& window);
  void end_frame(const GameWindow& window);

  bool ProcessEventSdl2(const SDL_Event& event);
};

} // namespace fightingengine
