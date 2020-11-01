
//header
#include "engine/core/game_window.hpp"

//c++ standard library headers
#include <iostream>

//other library headers
#include <SDL2/SDL_syswm.h>
#include <spdlog/spdlog.h>
#include <GL/glew.h>

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

        // Initialize SDL -----------------------

        if (SDL_WasInit(0) == 0)
        {
            SDL_SetMainReady();
            if (SDL_Init(0) != 0)
                throw std::runtime_error("Could not initialize SDL: " + std::string(SDL_GetError()));

            if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
                throw std::runtime_error("Could not initialize SDL Video Subsystem: " + std::string(SDL_GetError()));

            if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0)
                throw std::runtime_error("Could not initialize SDL Timer Subsystem: " + std::string(SDL_GetError()));
            
            if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
                throw std::runtime_error("Could not initialize SDL JoyStick Subsystem: " + std::string(SDL_GetError()));        
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

        // OpenGL--------------------------------------

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

            // //Check OpenGL
            // GLenum error;
            // while ((error = glGetError()) != GL_NO_ERROR)
            // {
            //     printf("ERROR GLEW: %s\n", gl_error_to_string(error));
            // }
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        //4, 3 because that's when compute shaders were introduced

        auto window_ptr = std::unique_ptr<SDL_Window, SDLDestroyer>(window);
        _window = std::move(window_ptr);
    }

    SDL_Window* GameWindow::get_handle() const
    {
        return _window.get();
    }

    bool GameWindow::is_open() const
    {
        return _window != nullptr;
    }


    uint32_t GameWindow::get_sdl_id() const
    {
        return SDL_GetWindowID(_window.get());
    }

    uint32_t GameWindow::get_sdl_flags() const
    {
        return SDL_GetWindowFlags(_window.get());
    }

    std::string GameWindow::get_title() const
    {
        return SDL_GetWindowTitle(_window.get());
    }

    void GameWindow::set_title(const std::string& str)
    {
        SDL_SetWindowTitle(_window.get(), str.c_str());
    }

    float GameWindow::get_aspect_ratio() const
    {
        int width, height;
        SDL_GetWindowSize(_window.get(), &width, &height);

        return (float)width / (float)height;
    }


    void GameWindow::set_position(int x, int y)
    {
        SDL_SetWindowPosition(_window.get(), x, y);
    }

    glm::ivec2 GameWindow::get_position() const
    {
        int x, y = 0;
        SDL_GetWindowPosition(_window.get(), &x, &y);
        return glm::ivec2(x, y);
    }

    void GameWindow::set_size(const glm::ivec2& size)
    {
        SDL_SetWindowSize(_window.get(), size.x, size.y);
    }

    glm::ivec2 GameWindow::get_size() const
    {
        int width, height = 0;
        SDL_GetWindowSize(_window.get(), &width, &height);
        return glm::ivec2(width, height);
    }

    void GameWindow::set_min_size(int width, int height)
    {
        SDL_SetWindowMinimumSize(_window.get(), width, height);
    }

    glm::ivec2 GameWindow::get_min_size() const
    {
        int width, height = 0;
        SDL_GetWindowMinimumSize(_window.get(), &width, &height);
        return glm::ivec2(width, height);
    }

    void GameWindow::set_max_size(int width, int height)
    {
        SDL_SetWindowMaximumSize(_window.get(), width, height);
    }

    glm::ivec2 GameWindow::get_max_size() const
    {
        int width, height = 0;
        SDL_GetWindowMaximumSize(_window.get(), &width, &height);
        return glm::ivec2(width, height);
    }


    void GameWindow::set_bordered(const bool b)
    {
        SDL_SetWindowBordered(_window.get(), b ? SDL_TRUE : SDL_FALSE);
    }

    void GameWindow::set_fullscreen(const bool b)
    {
        // todo: use DisplayMode
        if (SDL_SetWindowFullscreen(_window.get(), b ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
        {
            throw std::runtime_error("SDL_SetWindowFullscreen Error: " + std::string(SDL_GetError()));
        }
    }

    bool GameWindow::get_fullscreen() const
    {
        Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
        return SDL_GetWindowFlags(_window.get()) & FullscreenFlag;
    }

    void GameWindow::toggle_fullscreen()
    {
        if(get_fullscreen())
            set_fullscreen(false);
        else
            set_fullscreen(true);
    }


    float GameWindow::get_brightness() const
    {
        return SDL_GetWindowBrightness(_window.get());
    }

    void GameWindow::set_brightness(const float brightness)
    {
        if (SDL_SetWindowBrightness(_window.get(), brightness))
        {
            throw std::runtime_error("SDL_SetWindowBrightness Error: " + std::string(SDL_GetError()));
        }
    }


    void GameWindow::set_mouse_position(int x, int y)
    {
        SDL_WarpMouseInWindow(_window.get(), x, y);
    }

    bool GameWindow::get_mouse_captured() const
    {
        return SDL_GetWindowGrab(_window.get()) != SDL_FALSE;
    }

    void GameWindow::set_mouse_captured(const bool b)
    {
        if(b)
        {
            SDL_SetWindowGrab(_window.get(), SDL_TRUE);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        } 
        else
        {
            SDL_SetWindowGrab(_window.get(), SDL_FALSE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }

    void GameWindow::toggle_mouse_capture()
    {
        new_grab = get_mouse_captured();
        if(!new_grab)
        {
            set_mouse_captured(true);
            new_grab = true;
        }
        else
        {
            set_mouse_captured(false);
            new_grab = false;
        }
        printf("(App) Mouse grabbed? : %d \n", new_grab);
    }

    glm::ivec2 GameWindow::get_relative_mouse_state()
    {
        int x, y = 0;

        if(new_grab)
        {
            SDL_GetRelativeMouseState(&x, &y); 
            new_grab = false;

            //throw away this huge jumping value
            x = 0;
            y = 0;
        }
        else
        {
            SDL_GetRelativeMouseState(&x, &y);
        }

        return glm::ivec2(x, y);
    }


    void GameWindow::show()
    {
        SDL_ShowWindow(_window.get());
    }

    void GameWindow::hide()
    {
        SDL_HideWindow(_window.get());
    }

    void GameWindow::close()
    {
        SDL_DestroyWindow(_window.get());
        SDL_Quit();

        _window.reset(nullptr);

        SDL_GL_DeleteContext(get_gl_context());
    }

    void GameWindow::maximise()
    {
        SDL_MaximizeWindow(_window.get());
    }

    void GameWindow::minimize()
    {
        SDL_MinimizeWindow(_window.get());
    }

    void GameWindow::restore()
    {
        SDL_RestoreWindow(_window.get());
    }

    void GameWindow::raise()
    {
        SDL_RaiseWindow(_window.get());
    }



    SDL_GLContext& GameWindow::get_gl_context()
    {
        return gl_context;
    }

    std::string GameWindow::get_glsl_version() const
    {
        return glsl_version;
    }

}
