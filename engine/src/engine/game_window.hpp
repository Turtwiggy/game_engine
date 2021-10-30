#pragma once

// other library headers
#include <SDL2/SDL.h>
#undef main // thanks SDL2?
#include <glm/glm.hpp>

// c++ standard lib headers
#include <cstdint>
#include <memory>
#include <string>

namespace engine {

enum class DisplayMode
{
  WINDOWED,
  BORDERLESS,
  FULLSCREEN,
  FULLSCREEN_BORDERLESS,
};

class GameWindow
{
  struct SDLDestroyer
  {
    void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
  };

public:
  inline static std::string glsl_version;
  // Note: 4, 3 when compute shaders were introduced
  inline static int opengl_major;
  inline static int opengl_minor;

private:
  // SDL2 window
  std::unique_ptr<SDL_Window, SDLDestroyer> window_ = nullptr;

  // OpenGL context
  SDL_GLContext gl_context;

  // This variable fixes the "mouse jerk" issue
  // when you capture a mouse. The first frame of
  // SDL_GetRelativeMouseState(x, y) seems to
  // return a huge value, throw it away for
  // first frame value and set it to 0
  bool new_grab = false;

public:
  GameWindow(const std::string& title, int width, int height, DisplayMode displaymode, bool vsync);

  [[nodiscard]] SDL_Window* get_handle() const;
  [[nodiscard]] uint32_t get_sdl_id() const;
  [[nodiscard]] uint32_t get_sdl_flags() const;

  void set_title(const std::string& str);
  void set_position(int x, int y);
  void set_size(const glm::ivec2& size);
  void set_min_size(int width, int height);
  void set_max_size(int width, int height);
  void set_bordered(const bool b);
  void set_fullscreen(const bool f);
  void set_brightness(const float bright);
  void set_icon(SDL_Surface* icon);

  [[nodiscard]] std::string get_title() const;
  [[nodiscard]] glm::ivec2 get_position() const;
  [[nodiscard]] glm::ivec2 get_size() const;
  [[nodiscard]] glm::ivec2 get_min_size() const;
  [[nodiscard]] glm::ivec2 get_max_size() const;
  [[nodiscard]] bool get_fullscreen() const;
  [[nodiscard]] float get_brightness() const;

  void toggle_fullscreen();

  // ---- mouse

  void set_mouse_position(int x, int y);
  void set_mouse_captured(const bool b);

  // 0, 0, is top left of monitor
  // x, y is bottom right of monitor
  [[nodiscard]] glm::ivec2 get_mouse_position();
  [[nodiscard]] glm::ivec2 get_relative_mouse_position();
  [[nodiscard]] bool get_mouse_captured() const;

  void toggle_mouse_capture();

  // ---- sdl2 commands
  void show();
  void hide();
  void close();
  void minimize();
  void maximise();
  void restore();
  void raise();

  // ---- opengl context in sdl2 window
  [[nodiscard]] SDL_GLContext& get_gl_context();
  [[nodiscard]] std::string get_glsl_version() const;

  // ---- opengl specific impls
  void set_vsync_opengl(const bool vs);
  [[nodiscard]] bool get_vsync_opengl() const;
};

} // namespace engine
