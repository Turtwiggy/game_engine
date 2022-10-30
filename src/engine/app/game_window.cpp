
// header
#include "engine/app/game_window.hpp"

// other library headers

#if defined(__EMSCRIPTEN__)
#include <SDL2/SDL_opengles2.h>
#include <emscripten.h>
#else
#include <GL/glew.h>
#endif
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

// c++ standard library headers
#include <iostream>
#include <string>
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
#include "psapi.h"
#include "windows.h"
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
#endif

namespace engine {

GameWindow::GameWindow(const std::string& title, int width, int height, DisplayMode displaymode, bool vsync)
{
  SDL_version compiledVersion, linkedVersion;
  SDL_VERSION(&compiledVersion);
  SDL_GetVersion(&linkedVersion);

  std::cout << "Initializing SDL..."
            << "\n";
  std::cout << "SDL Version/Compiled " << unsigned(compiledVersion.major) << "." << unsigned(compiledVersion.minor)
            << "." << unsigned(compiledVersion.patch) << "\n";
  std::cout << "SDL Version/Linked " << unsigned(linkedVersion.major) << "." << unsigned(linkedVersion.minor) << "."
            << unsigned(linkedVersion.patch) << "\n";

  // Initialize SDL -----------------------

  if (SDL_WasInit(0) == 0) {
    SDL_SetMainReady();

    if (SDL_Init(0) != 0)
      std::cout << "could not initialize SDL: " << SDL_GetError();

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
      std::cout << "Could not initialize SDL Audio Subsystem:" << SDL_GetError() << "\n";

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
      std::cout << "Could not initialize SDL Video Subsystem:" << SDL_GetError() << "\n";

    if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
      std::cout << "Could not initialize SDL Timer Subsystem:" << SDL_GetError() << "\n";

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
      std::cout << "Could not initialize SDL JoyStick Subsystem:" << SDL_GetError() << "\n";
  }

  int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_ALLOW_HIGHDPI;
  flags |= SDL_WINDOW_RESIZABLE;

  if (displaymode == DisplayMode::borderless)
    flags |= SDL_WINDOW_BORDERLESS;
  else if (displaymode == DisplayMode::fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN;
  else if (displaymode == DisplayMode::fullscreen_borderless)
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  // Get SDL Window requirements from Renderer
  const int x = SDL_WINDOWPOS_UNDEFINED;
  const int y = SDL_WINDOWPOS_UNDEFINED;

  // OpenGL--------------------------------------

// emscripten
#if defined(__EMSCRIPTEN__)
  // webgl 1
  //
  // opengl_major = 2;
  // opengl_minor = 0;
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);

  // webgl 2

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  opengl_major = 3;
  opengl_minor = 0;
// mac
#elif defined(SDL_VIDEO_DRIVER_COCOA)
  opengl_major = 3;
  opengl_minor = 3;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

// windows
#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
  // opengl_major = 3;
  // opengl_minor = 3;
  // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // TEMPORARY: use opengl 3.0 es
  opengl_major = 3;
  opengl_minor = 0;
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, opengl_major);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, opengl_minor);

  // SDL2 Create Window --------------------------------------

  SDL_Window* window = SDL_CreateWindow(title.c_str(), x, y, width, height, flags);
  if (window == nullptr)
    std::cerr << "Failed to create SDL2 window: " << SDL_GetError() << "\n";

  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);

  if (gl_context == NULL) {
    printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
  } else {
#if !defined(__EMSCRIPTEN__)
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
      printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }
#endif
  }

  // Settings...
  SDL_SetWindowMinimumSize(window, 500, 300);
  SDL_ShowCursor(SDL_ENABLE);
  SDL_SetRelativeMouseMode(SDL_FALSE);
  set_vsync_opengl(vsync);

  auto window_ptr = std::unique_ptr<SDL_Window, SDLDestroyer>(window);
  window_ = std::move(window_ptr);
}

SDL_Window*
GameWindow::get_handle() const
{
  return window_.get();
}

void
GameWindow::get_native_handles(void*& native_window) const
{
#if defined(__EMSCRIPTEN__)
  static const char* canvas = "#canvas";
  native_window = const_cast<void*>(reinterpret_cast<const void*>(canvas));
  return;
#endif

  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  if (!SDL_GetWindowWMInfo(this->get_handle(), &wmi)) {
    std::cerr << "Failed getting native window handles: : " << std::string(SDL_GetError()) << "\n";
    exit(0);
  }

  // windows
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
  if (wmi.subsystem == SDL_SYSWM_WINDOWS) {
    native_window = wmi.info.win.window;
  } else
#endif // defined(SDL_VIDEO_DRIVER_WINDOWS)

  // Mac
#if defined(SDL_VIDEO_DRIVER_COCOA)
  {
  }
#endif // defined(SDL_VIDEO_DRIVER_COCOA)

  {
    std::cerr << "Unsupported platform: " << std::to_string(wmi.subsystem) << "\n";
    exit(0);
  }
};

uint32_t
GameWindow::get_sdl_id() const
{
  return SDL_GetWindowID(window_.get());
}

uint32_t
GameWindow::get_sdl_flags() const
{
  return SDL_GetWindowFlags(window_.get());
}

std::string
GameWindow::get_title() const
{
  return SDL_GetWindowTitle(window_.get());
}

void
GameWindow::set_title(const std::string& str)
{
  SDL_SetWindowTitle(window_.get(), str.c_str());
}

void
GameWindow::set_position(int x, int y)
{
  SDL_SetWindowPosition(window_.get(), x, y);
}

glm::ivec2
GameWindow::get_position() const
{
  int x, y = 0;
  SDL_GetWindowPosition(window_.get(), &x, &y);
  return glm::ivec2(x, y);
}

void
GameWindow::set_size(const glm::ivec2& size)
{
  SDL_SetWindowSize(window_.get(), size.x, size.y);
}

glm::ivec2
GameWindow::get_size() const
{
  int width, height = 0;
  SDL_GetWindowSize(window_.get(), &width, &height);
  return glm::ivec2(width, height);
}

void
GameWindow::set_min_size(int width, int height)
{
  SDL_SetWindowMinimumSize(window_.get(), width, height);
}

glm::ivec2
GameWindow::get_min_size() const
{
  int width, height = 0;
  SDL_GetWindowMinimumSize(window_.get(), &width, &height);
  return glm::ivec2(width, height);
}

void
GameWindow::set_max_size(int width, int height)
{
  SDL_SetWindowMaximumSize(window_.get(), width, height);
}

glm::ivec2
GameWindow::get_max_size() const
{
  int width, height = 0;
  SDL_GetWindowMaximumSize(window_.get(), &width, &height);
  return glm::ivec2(width, height);
}

void
GameWindow::set_bordered(const bool b)
{
  SDL_SetWindowBordered(window_.get(), b ? SDL_TRUE : SDL_FALSE);
}

void
GameWindow::set_fullscreen(const bool b)
{
  // todo: use DisplayMode
  if (SDL_SetWindowFullscreen(window_.get(), b ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)) {
    throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
  }
}

bool
GameWindow::get_fullscreen() const
{
  Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
  return SDL_GetWindowFlags(window_.get()) & FullscreenFlag;
}

void
GameWindow::toggle_fullscreen()
{
  set_fullscreen(!get_fullscreen());
}

float
GameWindow::get_brightness() const
{
  return SDL_GetWindowBrightness(window_.get());
}

void
GameWindow::set_brightness(const float brightness)
{
  if (SDL_SetWindowBrightness(window_.get(), brightness)) {
    throw std::runtime_error("SDL_SetWindowBrightness Error: " + std::string(SDL_GetError()));
  }
}

void
GameWindow::set_mouse_position(int x, int y)
{
  SDL_WarpMouseInWindow(window_.get(), x, y);
}

bool
GameWindow::get_mouse_captured() const
{
  return SDL_GetWindowGrab(window_.get()) != SDL_FALSE;
}

void
GameWindow::set_mouse_captured(const bool b)
{
  if (b) {
    SDL_SetWindowGrab(window_.get(), SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
  } else {
    SDL_SetWindowGrab(window_.get(), SDL_FALSE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
  }
}

void
GameWindow::toggle_mouse_capture()
{
  new_grab = get_mouse_captured();
  if (!new_grab) {
    set_mouse_captured(true);
    new_grab = true;
  } else {
    set_mouse_captured(false);
    new_grab = false;
  }
  printf("(App) Mouse grabbed? : %d \n", new_grab);
}

glm::ivec2
GameWindow::get_mouse_position()
{
  int x, y = 0;
  SDL_GetMouseState(&x, &y);
  return glm::ivec2(x, y);
}

glm::ivec2
GameWindow::get_relative_mouse_position()
{
  int x, y = 0;

  if (new_grab) {
    SDL_GetRelativeMouseState(&x, &y);
    new_grab = false;

    // throw away this huge jumping value
    x = 0;
    y = 0;
  } else {
    SDL_GetRelativeMouseState(&x, &y);
  }

  return glm::ivec2(x, y);
}

void
GameWindow::show()
{
  SDL_ShowWindow(window_.get());
}

void
GameWindow::hide()
{
  SDL_HideWindow(window_.get());
}

void
GameWindow::close()
{
  SDL_DestroyWindow(window_.get());
  SDL_Quit();

  window_.reset(nullptr);

  SDL_GL_DeleteContext(get_gl_context());
}

void
GameWindow::maximise()
{
  SDL_MaximizeWindow(window_.get());
}

void
GameWindow::minimize()
{
  SDL_MinimizeWindow(window_.get());
}

void
GameWindow::restore()
{
  SDL_RestoreWindow(window_.get());
}

void
GameWindow::raise()
{
  SDL_RaiseWindow(window_.get());
}

SDL_GLContext&
GameWindow::get_gl_context()
{
  return gl_context;
}

void
GameWindow::set_vsync_opengl(const bool vs)
{
  SDL_GL_SetSwapInterval(vs);
}

bool
GameWindow::get_vsync_opengl() const
{
  int enabled = SDL_GL_GetSwapInterval();
  return static_cast<bool>(enabled);
}
}
