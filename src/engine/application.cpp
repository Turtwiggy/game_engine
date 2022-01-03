
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
  std::string kBuildStr = "";

#ifdef _DEBUG
  kBuildStr = kBuildStr + std::string(" [DEBUG]");
#endif

  window = std::make_unique<GameWindow>(name + kBuildStr, width, height, DisplayMode::WINDOWED, vsync);

  imgui_manager.initialize(window.get());

  running = true;
}

Application::~Application()
{
  window->close();
}

bool
Application::is_running() const
{
  return running;
}

void
Application::shutdown()
{
  running = false;
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
  frame += 1;
  if (frame >= std::numeric_limits<uint64_t>::max())
    frame = 0;
  input_manager.new_frame(frame);

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
      // continue; // Imgui stole the event
    }

    // audio
    if (e.type == SDL_AUDIODEVICEADDED)
      std::cout << "audio device added?" << std::endl;
    if (e.type == SDL_AUDIODEVICEREMOVED)
      std::cout << "audio device removed?" << std::endl;

    // keyboard specific
    if (e.type == SDL_KEYDOWN)
      input_manager.process_key_down(e.key.keysym.scancode, e.key.repeat);
    if (e.type == SDL_KEYUP)
      input_manager.process_key_up(e.key.keysym.scancode, e.key.repeat);

    // controller specific
    // if (e.type == SDL_JOYHATMOTION)
    //   input_manager.process_controller_dpad(e.jhat);
    // if (e.type == SDL_JOYBUTTONDOWN)
    //   input_manager.process_controller_button_down(e.jbutton);
    // if (e.type == SDL_JOYBUTTONUP)
    //   input_manager.process_controller_button_up(e.jbutton);
    if (e.type == SDL_JOYDEVICEADDED)
      input_manager.process_controller_added();
    if (e.type == SDL_JOYDEVICEREMOVED)
      input_manager.process_controller_removed();

    // mouse specific
    if (e.type == SDL_MOUSEBUTTONDOWN) {
      input_manager.process_mouse_event(e.button);
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
Application::on_window_resize(int new_w, int new_h)
{
  printf("application resize event recieved: %i %i ", new_w, new_h);
  window_was_resized = true;

  if (new_w == 0 || new_h == 0) {
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