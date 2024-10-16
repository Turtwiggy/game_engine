
// header
#include "engine/app/game_window.hpp"
#include "engine/deps/opengl.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>
#include <format>

#include <stdexcept>
#include <string>

#if defined(SDL_VIDEO_DRIVER_WINDOWS)
#include "windows.h"
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
#endif

namespace engine {
using namespace std::literals;

GameWindow::GameWindow(const std::string& title, const DisplayMode& displaymode, const bool& vsync)
{
  SDL_version compiledVersion, linkedVersion;
  SDL_VERSION(&compiledVersion);
  SDL_GetVersion(&linkedVersion);

#if !defined(__EMSCRIPTEN__)
  setvbuf(stdout, nullptr, _IONBF, 0); // dont buffer fmt::println
#endif

  SDL_Log("%s", std::format("Initializing SDL...").c_str());
  SDL_Log("%s",
          std::format("SDL Version/Compiled {}.{}.{}", compiledVersion.major, compiledVersion.major, compiledVersion.patch)
            .c_str());
  SDL_Log("%s",
          std::format("SDL Version/Linked {}.{}.{}", linkedVersion.major, linkedVersion.major, linkedVersion.patch).c_str());

  // Initialize SDL -----------------------

  if (SDL_WasInit(0) == 0) {
    SDL_SetMainReady();

    if (SDL_Init(0) != 0)
      SDL_Log("%s", std::format("could not initialize SDL: %s", SDL_GetError()).c_str());

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
      SDL_Log("%s", std::format("Could not initialize SDL Audio Subsystem: %s", SDL_GetError()).c_str());

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
      SDL_Log("%s", std::format("Could not initialize SDL Video Subsystem: %s", SDL_GetError()).c_str());

    if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
      SDL_Log("%s", std::format("Could not initialize SDL Timer Subsystem: %s", SDL_GetError()).c_str());

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
      SDL_Log("%s", std::format("Could not initialize SDL JoyStick Subsystem: %s", SDL_GetError()).c_str());
  }

  // Get SDL Window requirements from Renderer
  const int x = SDL_WINDOWPOS_UNDEFINED;
  const int y = SDL_WINDOWPOS_UNDEFINED;

  // OpenGL--------------------------------------
  // OpenGL 3.0: #version 130
  // OpenGL 3.1: #version 140
  // OpenGL 3.2: #version 150
  // OpenGL 3.3: #version 330 core
  // OpenGL ES 2.0: #version 100
  // OpenGL ES 3.0: #version 300 es
  // OpenGL ES 3.1: #version 310 es
  // OpenGL ES 3.2: #version 320 es

// emscripten
#if defined(__EMSCRIPTEN__)
  // webgl 2
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
// mac
#elif defined(__APPLE__)
  // GL 3.3 + GLSL 330 core
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#else
  // GL 3.3 + GLSL 330 core
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // SDL2 Create Window --------------------------------------

  constexpr int tmp_h = 720;
  constexpr int tmp_w = 1280;

  int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_ALLOW_HIGHDPI;
  flags |= SDL_WINDOW_RESIZABLE;
  if (displaymode == DisplayMode::windowed_borderless)
    flags |= SDL_WINDOW_BORDERLESS;
  else if (displaymode == DisplayMode::fullscreen)
    flags |= SDL_WINDOW_FULLSCREEN;
  else if (displaymode == DisplayMode::fullscreen_borderless)
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

  SDL_Window* window = SDL_CreateWindow(title.c_str(), x, y, tmp_w, tmp_h, flags);
  if (window == nullptr)
    SDL_Log("%s", std::format("Failed to create SDL2 window: {}", SDL_GetError()).c_str());

  gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);

  if (gl_context == NULL) {
    SDL_Log("%s", std::format("OpenGL context could not be created! SDL Error: {}", SDL_GetError()).c_str());

  } else {
#if !defined(__EMSCRIPTEN__)
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
      const GLubyte* err = glewGetErrorString(glewError);
      SDL_Log("%s", std::format("Error initializing GLEW! {}", reinterpret_cast<const char*>(err)).c_str());
    }
#endif
  }

  // Settings...
  SDL_RaiseWindow(window);
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
    SDL_Log("%s", std::format("Failed getting native window handles: %s", SDL_GetError()).c_str());

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
    SDL_Log("%s", std::format("Unsupported platform: %s", std::to_string(wmi.subsystem)).c_str());
    exit(0);
  }
};

// OpenGL--------------------------------------
// OpenGL 3.0: #version 130
// OpenGL 3.1: #version 140
// OpenGL 3.2: #version 150
// OpenGL 3.3: #version 330 core
// OpenGL ES 2.0: #version 100
// OpenGL ES 3.0: #version 300 es
// OpenGL ES 3.1: #version 310 es
// OpenGL ES 3.2: #version 320 es`
std::string
GameWindow::get_glsl_version()
{
#if defined(__EMSCRIPTEN__)
  return "#version 300 es"s;
#else
  return "#version 330 core"s;
#endif
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
  int x = 0, y = 0;
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
  int width = 0, height = 0;
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
  int width = 0, height = 0;
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
  int width = 0, height = 0;
  SDL_GetWindowMaximumSize(window_.get(), &width, &height);
  return glm::ivec2(width, height);
}

glm::ivec2
GameWindow::get_resolution() const
{
  SDL_DisplayMode DM;
  SDL_GetCurrentDisplayMode(0, &DM);
  return { DM.w, DM.h };
}

void
GameWindow::set_displaymode(const DisplayMode mode)
{

  // set state
  switch (mode) {

    case DisplayMode::fullscreen: {
      if (SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN))
        throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
      break;
    }
    case DisplayMode::fullscreen_borderless: {
      SDL_SetWindowBordered(window_.get(), SDL_FALSE); // Ensure the window is borderless
      // This makes the window fullscreen but keeps the desktop resolution and runs the window in a
      // borderless, fullscreen mode.
      if (SDL_SetWindowFullscreen(window_.get(), SDL_WINDOW_FULLSCREEN_DESKTOP))
        throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
      SDL_SetWindowPosition(window_.get(), 0, 0);
      break;
    }
    case DisplayMode::windowed_borderless: {
      if (SDL_SetWindowFullscreen(window_.get(), 0))
        throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
      SDL_SetWindowBordered(window_.get(), SDL_FALSE);
      break;
    }
    case DisplayMode::windowed: {
      if (SDL_SetWindowFullscreen(window_.get(), 0))
        throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
      SDL_SetWindowBordered(window_.get(), SDL_TRUE);
      break;
    }
    default:
      break;
  }
};

bool
GameWindow::get_fullscreen() const
{
  Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
  return SDL_GetWindowFlags(window_.get()) & FullscreenFlag;
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
  SDL_Log("%s", std::format("(App) Mouse grabbed? : {}", new_grab).c_str());
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
  const int enabled = SDL_GL_GetSwapInterval();
  return static_cast<bool>(enabled);
}

void
GameWindow::set_icon(const std::string& path)
{
  // SDL_Surface* iconSurface = IMG_Load("icon.png"); // If PNG or other format
  // const SDL_Surface* iconSurface = SDL_LoadBMP(path);
  // SDL_SetWindowIcon(this.get, SDL_Surface * icon)

#if defined(WIN32) || defined(_WIN32)

  // Example: Setting the window icon
  HINSTANCE hInstance = GetModuleHandle(NULL);

  SDL_SysWMinfo wmi;
  SDL_VERSION(&wmi.version);
  if (!SDL_GetWindowWMInfo(this->get_handle(), &wmi))
    return;

  HWND hWnd = wmi.info.win.window;
  HICON hIcon = (HICON)LoadImage(NULL,         // hInstance is NULL when loading from a file
                                 path.c_str(), // Path to the icon file
                                 IMAGE_ICON,
                                 32, // Icon size (width, height)
                                 32,
                                 LR_LOADFROMFILE | LR_DEFAULTSIZE // Load icon from file and use default size
  );

  if (hIcon) {
    SDL_Log("%s", std::format("setting icon...").c_str());
    SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
  } else
    SDL_Log("%s", std::format("unable to load icon...").c_str());

    //
#endif
}

} // namespace game2d
