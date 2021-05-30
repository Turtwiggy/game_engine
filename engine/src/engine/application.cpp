
// header
#include "engine/application.hpp"

// c++ standard lib headers
#include <iostream>
#include <numeric>

// other lib headers
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

namespace fightingengine {

Application::Application(const std::string& name, int width, int height)
{
  // const std::string kBuildStr(kGitSHA1Hash, 8);
  const std::string kBuildStr = "0.0.4";

  // Window
  GameWindow::glsl_version = "#version 330";
  GameWindow::opengl_major = 3;
  GameWindow::opengl_minor = 3;
  window = std::make_unique<GameWindow>(name + " [" + kBuildStr + "]", width, height, DisplayMode::BORDERLESS);

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
  // now = SDL_GetTicks();         //Returns an unsigned 32-bit value
  // representing the number of milliseconds since the SDL library initialized.
  // uint32_t delta_time_in_milliseconds = now - prev;
  // if (delta_time_in_milliseconds < 0) return 0; prev = now;

  return ImGui::GetIO().DeltaTime;
}

void
Application::frame_end(const float delta_time)
{
  imgui_manager.end_frame(get_window());

  SDL_GL_SwapWindow(get_window().get_handle());

  // // If frame finished early
  // float delta_time_ms = delta_time * 1000.0f;
  // std::cout << "deltatimems: " << delta_time_ms << " mspf: " << MILLISECONDS_PER_FRAME << std::endl;
  
  // if (fps_limit && delta_time_ms < MILLISECONDS_PER_FRAME){
  //   Uint32 delay = static_cast<Uint32>(MILLISECONDS_PER_FRAME) - static_cast<Uint32>(delta_time_ms);
  //   // std::cout << "delaying! " << delay << std::endl;
  //   SDL_Delay(delay);
  // }
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

void 
Application::set_fps_limit(const float fps)
{
  fps_limit = true;
  // FPS = fps;
  // MILLISECONDS_PER_FRAME = (1.0f / fps) * 1000;
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

ImGui_Manager&
Application::get_imgui()
{
  return imgui_manager;
}

GameWindow&
Application::get_window()
{
  return *window;
}
}
