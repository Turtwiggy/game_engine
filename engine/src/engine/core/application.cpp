
// header
#include "engine/core/application.hpp"

// c++ standard lib headers
#include <numeric>

// other lib headers
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

namespace fightingengine {

const std::string kBuildStr = "1";
// const std::string kBuildStr(kGitSHA1Hash, 8);

Application::Application(const std::string& name, int width, int height)
{
  // Window
  window = std::make_unique<GameWindow>(name + " [" + kBuildStr + "]", width, height, DisplayMode::Windowed);

  fps_buffer = boost::circular_buffer<float>(100);

  imgui_manager.initialize(window.get());

  running = true;
  start = now = SDL_GetTicks();
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

  // float delta_time_in_seconds = (delta_time_in_milliseconds / 1000.f);
  // time_since_launch += delta_time_in_seconds;
  // printf("delta_time %f \n", delta_time_in_seconds);

  return ImGui::GetIO().DeltaTime;
}

float
Application::get_time_since_launch() const
{
  return time_since_launch;
}

void
Application::frame_begin()
{
  input_manager.new_frame();

  time_since_launch += get_delta_time();
}

void
Application::frame_end(const float delta_time)
{
  // FPS Profiling
  // -------------
  fps_buffer.push_back(1.f / delta_time);

  SDL_GL_SwapWindow(get_window().get_handle());

  // If frame finished early
  if (fps_limit && delta_time < MILLISECONDS_PER_FRAME)
    SDL_Delay(MILLISECONDS_PER_FRAME - delta_time);
}

void
Application::poll()
{
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (imgui_manager.ProcessEventSdl2(e)) {
      // Imgui stole the event
      continue;
    }

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

    // int mouse_x, mouse_y;
    // SDL_GetMouseState(&mouse_x, &mouse_y);

    // Key events
    switch (e.type) {
      case SDL_KEYDOWN:
        // keyboard specific
        {
          SDL_KeyboardEvent key_event = e.key;
          auto key = key_event.keysym.sym;
          input_manager.add_button_down(key);
          break;
        }
      case SDL_MOUSEBUTTONDOWN:
        // mouse specific
        {
          input_manager.add_mouse_down(e.button);
          break;
        }

      case SDL_MOUSEWHEEL:
        // mouse scrollwheel
        {
          input_manager.set_mousewheel_y(static_cast<float>(e.wheel.y));
          break;
        }
    }
  }
}

void
Application::gui_begin()
{
  imgui_manager.begin_frame(get_window());
}

void
Application::gui_end()
{
  imgui_manager.end_frame(get_window());
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

void
Application::set_fps_limit(const float fps)
{
  fps_limit = true;
  FPS = fps;
  MILLISECONDS_PER_FRAME = (int)(1000 / FPS);
}

void
Application::remove_fps_limit()
{
  fps_limit = false;
}

float
Application::get_average_fps() const
{
  return std::accumulate(fps_buffer.begin(), fps_buffer.end(), 0) / static_cast<float>(fps_buffer.size());
}

float
Application::get_raw_fps(const float delta_time) const
{
  return 1.f / delta_time;
}

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
