
// header
#include "engine/application.hpp"

// other lib headers
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

// c++ standard lib headers
#include <iostream>
#include <numeric>

namespace engine {

Application::Application(const std::string& name, int width, int height, bool vsync)
{
  // const std::string kBuildStr(kGitSHA1Hash, 8);
  std::string kBuildStr = " [0.0.8] ";

#ifdef WIN32
#ifdef _DEBUG
  kBuildStr = kBuildStr + std::string("[DEBUG]");
#endif
#endif

  // Window
  GameWindow::glsl_version = "#version 330";
  GameWindow::opengl_major = 3;
  GameWindow::opengl_minor = 3;
  window = std::make_unique<GameWindow>(name + kBuildStr, width, height, DisplayMode::WINDOWED, vsync);

  imgui_manager.initialize(window.get());

  running = true;
}

Application::~Application()
{
  window->close();
}

void
Application::shutdown()
{
  running = false;
}

bool
Application::is_running() const
{
  return running;
}

float
Application::get_delta_time()
{
  return ImGui::GetIO().DeltaTime;
}

void
Application::frame_end(Uint64& frame_start_time)
{
  imgui_manager.end_frame(get_window());

  SDL_GL_SwapWindow(get_window().get_handle());

  Uint64 frame_end_time = SDL_GetPerformanceCounter();
  float elapsed_ms = (frame_end_time - frame_start_time) / static_cast<float>(SDL_GetPerformanceFrequency()) * 1000.0f;

  if (limit_fps) {
    auto delay = floor(1000.0f / fps_if_limited - elapsed_ms);
    if (delay > 0.0f)
      SDL_Delay(static_cast<Uint32>(delay));
  }
}

void
Application::frame_begin()
{
  input_manager.new_frame();

  SDL_Event e;
  while (SDL_PollEvent(&e)) {

    // Events to quit
    if (e.type == SDL_QUIT) {
      on_window_close();
    }

    // https://wiki.libsdl.org/SDL_WindowEvent
    if (e.type == SDL_WINDOWEVENT) {
      switch (e.window.event) {
        case SDL_WINDOWEVENT_CLOSE:
          if (e.window.windowID == SDL_GetWindowID(window->get_handle())) {
            on_window_close();
          }
          break;
        case SDL_WINDOWEVENT_RESIZED:
          if (e.window.windowID == SDL_GetWindowID(window->get_handle())) {
            on_window_resize(e.window.data1, e.window.data2);
          }
          break;
        case SDL_WINDOWEVENT_MINIMIZED:
          if (e.window.windowID == SDL_GetWindowID(window->get_handle())) {
            minimized = true;
          }
          break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
          SDL_Log("Window %d gained keyboard focus \n", e.window.windowID);
          break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
          SDL_Log("Window %d lost keyboard focus \n", e.window.windowID);
          break;
      }
    }

    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
      imgui_manager.process_event(&e);
      continue; // Imgui stole the event
    }

    // mouse specific
    if (e.type == SDL_MOUSEBUTTONDOWN) {
      input_manager.add_mouse_down(e.button);
    }
    // mouse scrollwheel
    if (e.type == SDL_MOUSEWHEEL) {
      input_manager.set_mousewheel_y(static_cast<float>(e.wheel.y));
    }
  }

  imgui_manager.begin_frame(get_window());
  seconds_since_launch += get_delta_time();
}

// ---- events

void
Application::on_window_close()
{
  printf("application close event recieved");
  running = false;
}

void
Application::on_window_resize(int w, int h)
{
  printf("application resize event recieved: %i %i ", w, h);
  window_was_resized = true;

  if (w == 0 || h == 0) {
    minimized = true;
    return;
  }

  minimized = false;
}

// ---- window controls

InputManager&
Application::get_input()
{
  return input_manager;
}

GameWindow&
Application::get_window()
{
  return *window;
}

}
