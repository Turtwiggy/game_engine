#include "game_window.hpp"
#include "engine/renderer/util/util_functions.hpp"

#include <SDL2/SDL_syswm.h>
#include <spdlog/spdlog.h>
#include <GL/glew.h>

#include <iostream>

namespace fightingengine {

    GameWindow::GameWindow(const std::string& title, const SDL_DisplayMode& display, display_mode displaymode)
        : GameWindow::GameWindow(title, display.w, display.h, displaymode)
    {
    }

    GameWindow::GameWindow(const std::string& title, int width, int height, display_mode displaymode)
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
            | SDL_WINDOW_ALLOW_HIGHDPI;

        if (displaymode == display_mode::Fullscreen)
            flags |= SDL_WINDOW_FULLSCREEN;
        else if (displaymode == display_mode::Borderless)
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

        // Get SDL Window requirements from Renderer
        const int x = SDL_WINDOWPOS_UNDEFINED;
        const int y = SDL_WINDOWPOS_UNDEFINED;

        SDL_Window* window = SDL_CreateWindow(title.c_str(), x, y, width, height, flags);
        if (window == nullptr)
        {
            spdlog::error("Failed to create SDL2 window: '{}'", SDL_GetError());
            throw std::runtime_error("Failed creating SDL2 window: " + std::string(SDL_GetError()));
        }

        SDL_SetWindowMinimumSize(window, 500, 300);
        SDL_GL_SetSwapInterval(0); //VSync
        SDL_ShowCursor(SDL_ENABLE);
        SDL_SetRelativeMouseMode(SDL_FALSE);

        // --------------------------------------------

        // OpenGL--------------------------------------

        // --------------------------------------------

        gl_context = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, gl_context);

        if (gl_context == NULL)
        {
            printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
            throw std::runtime_error("Failed creating SDL2 window: " + std::string(SDL_GetError()));
        }
        else
        {
            //Initialize GLEW
            glewExperimental = GL_TRUE;
            GLenum glewError = glewInit();
            if (glewError != GLEW_OK)
            {
                printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
                throw std::runtime_error("Error initializing GLEW! " + std::string(SDL_GetError()));
            }

            //Check OpenGL
            GLenum error;
            while ((error = glGetError()) != GL_NO_ERROR)
            {
                printf("ERROR GLEW: %s\n", gl_error_to_string(error));
            }
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        //4, 3 because that's when compute shaders were introduced

        auto window_ptr = std::unique_ptr<SDL_Window, SDLDestroyer>(window);
        _window = std::move(window_ptr);
    }

    SDL_Window* GameWindow::GetHandle() const
    {
        return _window.get();
    }

    // void GameWindow::GetNativeHandles(void*& native_window, void*& native_display) const
//     {
//         SDL_SysWMinfo wmi;
//         SDL_VERSION(&wmi.version);
//         if (!SDL_GetWindowWMInfo(_window.get(), &wmi))
//         {
//             throw std::runtime_error("Failed getting native window handles: " + std::string(SDL_GetError()));
//         }
//         // Linux
// #if defined(SDL_VIDEO_DRIVER_WAYLAND)
//         if (wmi.subsystem == SDL_SYSWM_WAYLAND)
//         {
//             auto win_impl = static_cast<wl_egl_window*>(SDL_GetWindowData(_window.get(), "wl_egl_window"));
//             if (!win_impl)
//             {
//                 int width, height;
//                 SDL_GetWindowSize(_window.get(), &width, &height);
//                 struct wl_surface* surface = wmi.info.wl.surface;
//                 if (!surface)
//                 {
//                     throw std::runtime_error("Failed getting native window handles: " + std::string(SDL_GetError()));
//                 }
//                 win_impl = wl_egl_window_create(surface, width, height);
//                 SDL_SetWindowData(_window.get(), "wl_egl_window", win_impl);
//             }
//             native_window = reinterpret_cast<void*>(win_impl);
//             native_display = wmi.info.wl.display;
//         }
//         else
// #endif // defined(SDL_VIDEO_DRIVER_WAYLAND)
// #if defined(SDL_VIDEO_DRIVER_X11)
//             if (wmi.subsystem == SDL_SYSWM_X11)
//             {
//                 native_window = reinterpret_cast<void*>(wmi.info.x11.window);
//                 native_display = wmi.info.x11.display;
//             }
//             else
// #endif // defined(SDL_VIDEO_DRIVER_X11) \
//        // Mac
// #if defined(SDL_VIDEO_DRIVER_COCOA)
//                 if (wmi.subsystem == SDL_SYSWM_COCOA)
//                 {
//                     native_window = wmi.info.cocoa.window;
//                     native_display = nullptr;
//                 }
//                 else
// #endif // defined(SDL_VIDEO_DRIVER_COCOA) \
//        // Windows
// #if defined(SDL_VIDEO_DRIVER_WINDOWS)
//                     if (wmi.subsystem == SDL_SYSWM_WINDOWS)
//                     {
//                         native_window = wmi.info.win.window;
//                         native_display = nullptr;
//                     }
//                     else
// #endif // defined(SDL_VIDEO_DRIVER_WINDOWS) \
//        // Steam Link
// #if defined(SDL_VIDEO_DRIVER_VIVANTE)
//                         if (wmi.subsystem == SDL_SYSWM_VIVANTE)
//                         {
//                             native_window = wmi.info.vivante.window;
//                             native_display = wmi.info.vivante.display;
//                         }
//                         else
// #endif // defined(SDL_VIDEO_DRIVER_VIVANTE)
//                         {
//                             throw std::runtime_error("Unsupported platform or window manager: " + std::to_string(wmi.subsystem));
//                         }
//     }

    bool GameWindow::IsOpen() const
    {
        return _window != nullptr;
    }

    float GameWindow::GetBrightness() const
    {
        return SDL_GetWindowBrightness(_window.get());
    }

    void GameWindow::SetBrightness(float brightness)
    {
        if (SDL_SetWindowBrightness(_window.get(), brightness))
        {
            throw std::runtime_error("SDL_SetWindowBrightness Error: " + std::string(SDL_GetError()));
        }
    }

    uint32_t GameWindow::GetID() const
    {
        return SDL_GetWindowID(_window.get());
    }

    uint32_t GameWindow::GetFlags() const
    {
        return SDL_GetWindowFlags(_window.get());
    }

    void GameWindow::GrabInput(bool b)
    {
        SDL_SetWindowGrab(_window.get(), b ? SDL_TRUE : SDL_FALSE);
    }

    void GameWindow::SetMousePosition(int x, int y)
    {
        SDL_WarpMouseInWindow(_window.get(), x, y);
    }

    bool GameWindow::IsInputGrabbed() const
    {
        return SDL_GetWindowGrab(_window.get()) != SDL_FALSE;
    }

    std::string GameWindow::GetTitle() const
    {
        return SDL_GetWindowTitle(_window.get());
    }

    void GameWindow::SetTitle(const std::string& str)
    {
        SDL_SetWindowTitle(_window.get(), str.c_str());
    }

    float GameWindow::GetAspectRatio() const
    {
        int width, height;
        SDL_GetWindowSize(_window.get(), &width, &height);

        return (float)width / (float)height;
    }

    void GameWindow::SetPosition(int x, int y)
    {
        SDL_SetWindowPosition(_window.get(), x, y);
    }

    void GameWindow::GetPosition(int& x, int& y) const
    {
        SDL_GetWindowPosition(_window.get(), &x, &y);
    }

    void GameWindow::SetMinimumSize(int width, int height)
    {
        SDL_SetWindowMinimumSize(_window.get(), width, height);
    }

    void GameWindow::GetMinimumSize(int& width, int& height) const
    {
        SDL_GetWindowMinimumSize(_window.get(), &width, &height);
    }

    void GameWindow::SetMaximumSize(int width, int height)
    {
        SDL_SetWindowMaximumSize(_window.get(), width, height);
    }

    void GameWindow::GetMaximumSize(int& width, int& height) const
    {
        SDL_GetWindowMaximumSize(_window.get(), &width, &height);
    }

    void GameWindow::SetSize(int width, int height)
    {
        SDL_SetWindowSize(_window.get(), width, height);
    }

    void GameWindow::GetSize(int& width, int& height) const
    {
        SDL_GetWindowSize(_window.get(), &width, &height);
    }

    void GameWindow::Show()
    {
        SDL_ShowWindow(_window.get());
    }

    void GameWindow::Hide()
    {
        SDL_HideWindow(_window.get());
    }

    void GameWindow::Maximise()
    {
        SDL_MaximizeWindow(_window.get());
    }

    void GameWindow::Minimise()
    {
        SDL_MinimizeWindow(_window.get());
    }

    void GameWindow::Restore()
    {
        SDL_RestoreWindow(_window.get());
    }

    void GameWindow::Raise()
    {
        SDL_RaiseWindow(_window.get());
    }

    void GameWindow::SetBordered(bool b)
    {
        SDL_SetWindowBordered(_window.get(), b ? SDL_TRUE : SDL_FALSE);
    }

    void GameWindow::SetFullscreen(bool b)
    {
        // todo: use DisplayMode
        if (SDL_SetWindowFullscreen(_window.get(), b ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
        {
            throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
        }
    }

    bool GameWindow::ToggleFullscreen()
    {
        Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
        bool IsFullscreen = SDL_GetWindowFlags(_window.get()) & FullscreenFlag;
        SDL_SetWindowFullscreen(_window.get(), IsFullscreen ? 0 : FullscreenFlag);
        //SDL_ShowCursor(IsFullscreen);

        return !IsFullscreen;
    }

    void GameWindow::Close()
    {
        SDL_DestroyWindow(_window.get());
        SDL_Quit();

        _window.reset(nullptr);

        SDL_GL_DeleteContext(get_gl_context());
    }

    SDL_GLContext& GameWindow::get_gl_context()
    {
        return gl_context;
    }

    std::string GameWindow::get_glsl_version()
    {
        return glsl_version;
    }

    //returns true if mouse is grabbed, false if it available
    void GameWindow::ToggleMouseCaptured()
    {
        bool grabbed = !IsInputGrabbed();
        SDL_SetRelativeMouseMode(grabbed ? SDL_TRUE : SDL_FALSE);
        GrabInput(grabbed);
    }
}
