#include "graphics/renderer.h"

#include "graphics/render_command.h"
#include "window/game_window.h"
#include "3d/fg_texture.hpp"
#include "util/util_functions.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <SDL2/SDL.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif
#include "stb_image.h"
#include "imgui.h"
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>

#include <vector>
#include <memory>

namespace fightinggame
{
    struct RenderData
    {
        //textures
        unsigned int wood_texture;
        unsigned int second_texture;
    };
    static RenderData s_Data;

    void Renderer::init_renderer(int screen_width, int screen_height)
    {
        // load textures
        // -------------
        s_Data.wood_texture = TextureFromFile("BambooWall_1K_albedo.jpg", "assets/textures/Bamboo", false);
        s_Data.second_texture = TextureFromFile("container.jpg", "assets/textures", false);

        //GPU Info
        printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));
        int params[1];
        glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, params);
        printf("GPU INFO: Max shader storage buffer bindings: %i \n", params[0]);
        //GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS = 96
        //GL_MAX_SHADER_STORAGE_BLOCK_SIZE = 2147483647
        //GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS = 16
        //GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS = 96

        // draw as wireframe
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void Renderer::new_frame(SDL_Window* window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);

        ImGui::SetCurrentContext(_imgui);
        ImGui::NewFrame();
    }

    void Renderer::end_frame(SDL_Window* window)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }

        SDL_GL_SwapWindow(window);
    }

    void Renderer::shutdown()
    {
        ImGui::SetCurrentContext(_imgui);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(get_gl_context());
    }

    void Renderer::init_opengl_and_imgui(const GameWindow& window)
    {
        //OpenGL
        gl_context = SDL_GL_CreateContext(window.GetHandle());
        SDL_GL_MakeCurrent(window.GetHandle(), gl_context);

        int width, height;
        window.GetSize(width, height);
        RenderCommand::set_viewport(0, 0, width, height);

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

        //Setup ImGui
        IMGUI_CHECKVERSION();
        _imgui = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        std::string glsl_version = "#version 430";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        //4, 3 because that's when compute shaders were introduced

        // setup platform/renderer bindings
        ImGui_ImplSDL2_InitForOpenGL(window.GetHandle(), gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());

        //configure opengl state
        RenderCommand::init();
    }
}
