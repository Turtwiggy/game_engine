#include "game_window.h"

#include <SDL2/SDL_syswm.h>
#include <spdlog/spdlog.h>
#include <iostream>

using namespace fightinggame;

game_window::game_window(const std::string& title, const SDL_DisplayMode& display, display_mode displaymode)
	: game_window::game_window(title, display.w, display.h, displaymode)
{
}

game_window::game_window(const std::string& title, int width, int height, display_mode displaymode)
{
	SDL_version compiledVersion, linkedVersion;
	SDL_VERSION(&compiledVersion);
	SDL_GetVersion(&linkedVersion);

	spdlog::info("Initializing SDL...");
	spdlog::info("SDL Version/Compiled {}.{}.{}", compiledVersion.major, compiledVersion.minor, compiledVersion.patch);
	spdlog::info("SDL Version/Linked {}.{}.{}", linkedVersion.major, linkedVersion.minor, linkedVersion.patch);

	// Initialize SDL
	if (SDL_WasInit(0) == 0)
	{
		SDL_SetMainReady();
		if (SDL_Init(0) != 0)
			throw std::runtime_error("Could not initialize SDL: " + std::string(SDL_GetError()));

		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
			throw std::runtime_error("Could not initialize SDL Video Subsystem: " + std::string(SDL_GetError()));

		if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
			throw std::runtime_error("Could not initialize SDL Timer Subsystem: " + std::string(SDL_GetError()));
	}

	int flags = SDL_WINDOW_OPENGL
		| SDL_WINDOW_INPUT_FOCUS 
		| SDL_WINDOW_RESIZABLE 
		| SDL_WINDOW_ALLOW_HIGHDPI ;
	if (displaymode == display_mode::Fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
	else if (displaymode == display_mode::Borderless)
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	// Get SDL Window requirements from Renderer
	const int x = SDL_WINDOWPOS_UNDEFINED;
	const int y = SDL_WINDOWPOS_UNDEFINED;

	auto window = std::unique_ptr<SDL_Window, SDLDestroyer>(SDL_CreateWindow(title.c_str(), x, y, width, height, flags));

	if (window == nullptr)
	{
		spdlog::error("Failed to create SDL2 window: '{}'", SDL_GetError());
		throw std::runtime_error("Failed creating SDL2 window: " + std::string(SDL_GetError()));
	}

	SDL_SetWindowMinimumSize(window.get(), 500, 300);
	SDL_GL_SetSwapInterval(0); //VSync
	SDL_ShowCursor(SDL_ENABLE);
	//SetMouseCaptured(true);
	//GrabInput(true);
	//SDL_SetWindowGrab(window.get(), SDL_TRUE);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	_window = std::move(window);
}

SDL_Window* game_window::GetHandle() const
{
	return _window.get();
}

void game_window::GetNativeHandles(void*& native_window, void*& native_display) const
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window.get(), &wmi))
	{
		throw std::runtime_error("Failed getting native window handles: " + std::string(SDL_GetError()));
	}

	// Linux
#if defined(SDL_VIDEO_DRIVER_WAYLAND)
	if (wmi.subsystem == SDL_SYSWM_WAYLAND)
	{
		auto win_impl = static_cast<wl_egl_window*>(SDL_GetWindowData(_window.get(), "wl_egl_window"));
		if (!win_impl)
		{
			int width, height;
			SDL_GetWindowSize(_window.get(), &width, &height);
			struct wl_surface* surface = wmi.info.wl.surface;
			if (!surface)
			{
				throw std::runtime_error("Failed getting native window handles: " + std::string(SDL_GetError()));
			}
			win_impl = wl_egl_window_create(surface, width, height);
			SDL_SetWindowData(_window.get(), "wl_egl_window", win_impl);
		}
		native_window = reinterpret_cast<void*>(win_impl);
		native_display = wmi.info.wl.display;
	}
	else
#endif // defined(SDL_VIDEO_DRIVER_WAYLAND)
#if defined(SDL_VIDEO_DRIVER_X11)
		if (wmi.subsystem == SDL_SYSWM_X11)
		{
			native_window = reinterpret_cast<void*>(wmi.info.x11.window);
			native_display = wmi.info.x11.display;
		}
		else
#endif // defined(SDL_VIDEO_DRIVER_X11) \
       // Mac
#if defined(SDL_VIDEO_DRIVER_COCOA)
			if (wmi.subsystem == SDL_SYSWM_COCOA)
			{
				native_window = wmi.info.cocoa.window;
				native_display = nullptr;
			}
			else
#endif // defined(SDL_VIDEO_DRIVER_COCOA) \
       // Windows
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
				if (wmi.subsystem == SDL_SYSWM_WINDOWS)
				{
					native_window = wmi.info.win.window;
					native_display = nullptr;
				}
				else
#endif // defined(SDL_VIDEO_DRIVER_WINDOWS) \
       // Steam Link
#if defined(SDL_VIDEO_DRIVER_VIVANTE)
					if (wmi.subsystem == SDL_SYSWM_VIVANTE)
					{
						native_window = wmi.info.vivante.window;
						native_display = wmi.info.vivante.display;
					}
					else
#endif // defined(SDL_VIDEO_DRIVER_VIVANTE)
					{
						throw std::runtime_error("Unsupported platform or window manager: " + std::to_string(wmi.subsystem));
					}
}

bool game_window::IsOpen() const
{
	return _window != nullptr;
}

float game_window::GetBrightness() const
{
	return SDL_GetWindowBrightness(_window.get());
}

void game_window::SetBrightness(float brightness)
{
	if (SDL_SetWindowBrightness(_window.get(), brightness))
	{
		throw std::runtime_error("SDL_SetWindowBrightness Error: " + std::string(SDL_GetError()));
	}
}

uint32_t game_window::GetID() const
{
	return SDL_GetWindowID(_window.get());
}

uint32_t game_window::GetFlags() const
{
	return SDL_GetWindowFlags(_window.get());
}

void game_window::GrabInput(bool b)
{
	SDL_SetWindowGrab(_window.get(), b ? SDL_TRUE : SDL_FALSE);
}

void game_window::SetMousePosition(int x, int y)
{
	SDL_WarpMouseInWindow(_window.get(), x, y);
}

bool game_window::IsInputGrabbed() const
{
	return SDL_GetWindowGrab(_window.get()) != SDL_FALSE;
}

std::string game_window::GetTitle() const
{
	return SDL_GetWindowTitle(_window.get());
}

void game_window::SetTitle(const std::string& str)
{
	SDL_SetWindowTitle(_window.get(), str.c_str());
}

float game_window::GetAspectRatio() const
{
	int width, height;
	SDL_GetWindowSize(_window.get(), &width, &height);

	return (float)width / (float)height;
}

void game_window::SetPosition(int x, int y)
{
	SDL_SetWindowPosition(_window.get(), x, y);
}

void game_window::GetPosition(int& x, int& y) const
{
	SDL_GetWindowPosition(_window.get(), &x, &y);
}

void game_window::SetMinimumSize(int width, int height)
{
	SDL_SetWindowMinimumSize(_window.get(), width, height);
}

void game_window::GetMinimumSize(int& width, int& height) const
{
	SDL_GetWindowMinimumSize(_window.get(), &width, &height);
}

void game_window::SetMaximumSize(int width, int height)
{
	SDL_SetWindowMaximumSize(_window.get(), width, height);
}

void game_window::GetMaximumSize(int& width, int& height) const
{
	SDL_GetWindowMaximumSize(_window.get(), &width, &height);
}

void game_window::SetSize(int width, int height)
{
	SDL_SetWindowSize(_window.get(), width, height);
}

void game_window::GetSize(int& width, int& height) const
{
	SDL_GetWindowSize(_window.get(), &width, &height);
}

void game_window::Show()
{
	SDL_ShowWindow(_window.get());
}

void game_window::Hide()
{
	SDL_HideWindow(_window.get());
}

void game_window::Maximise()
{
	SDL_MaximizeWindow(_window.get());
}

void game_window::Minimise()
{
	SDL_MinimizeWindow(_window.get());
}

void game_window::Restore()
{
	SDL_RestoreWindow(_window.get());
}

void game_window::Raise()
{
	SDL_RaiseWindow(_window.get());
}

void game_window::SetBordered(bool b)
{
	SDL_SetWindowBordered(_window.get(), b ? SDL_TRUE : SDL_FALSE);
}

void game_window::SetFullscreen(bool b)
{
	// todo: use DisplayMode
	if (SDL_SetWindowFullscreen(_window.get(), b ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
	{
		throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
	}
}

void game_window::Close()
{
	_window.reset(nullptr);
}

void game_window::SetMouseCaptured(bool b)
{
	SDL_CaptureMouse( b ? SDL_TRUE : SDL_FALSE);
}