#include "graphics/renderer.h"

#include "data/shapes.h"
#include "graphics/render_command.h"
#include "graphics/texture_manager.h"
#include "window/game_window.h"

#include <GL/glew.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <SDL2/SDL.h>
#ifdef _WIN32
#include <SDL2/SDL_syswm.h>
#endif
#include "stb_image.h"
#include "imgui.h"
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>

using namespace fightinggame;
using namespace fightinggame::graphics;

namespace fightinggame
{
    struct RenderData
    {
        // TODO: RenderCaps
        static const uint32_t MaxModels = 20;

        std::shared_ptr<Shader> diffuse_shader;
        std::shared_ptr<Shader> terrain_shader;

        Renderer::Statistics stats;
    };
    static RenderData s_Data;

    void Renderer::init_renderer()
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // uncomment this call to draw in wireframe polygons.

        //Diffuse Shader
        s_Data.diffuse_shader = std::make_unique<Shader>
            ("assets/shaders/lit_directional.vert", "assets/shaders/lit_directional.frag");

        ////Lit Object Shader
        s_Data.terrain_shader = std::make_unique<Shader>
            ("assets/shaders/terrain.vert", "assets/shaders/terrain.frag");

        //Enable Multi Sampling
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        glEnable(GL_MULTISAMPLE);

        //Enable Faceculling
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Enable depth testing
        glEnable(GL_DEPTH_TEST);

        //Create our own framebuffer
        //unsigned int framebuffer;
        //glGenFramebuffers(1, &framebuffer);
        //glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        //// generate texture
        //unsigned int texColorBuffer;
        //glGenTextures(1, &texColorBuffer);
        //glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glBindTexture(GL_TEXTURE_2D, 0);

        //// attach it to currently bound framebuffer object
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
        //if (glCheckFramebufferStatus(framebuffer) == GL_FRAMEBUFFER_COMPLETE)
        //{
        //}
        //glDeleteFramebuffers(1, &framebuffer);
    }

    void Renderer::draw_pass(draw_scene_desc& desc, GameState state)
    {
        RenderCommand::set_clear_colour(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        RenderCommand::clear();

        int width, height = 0;
        GameWindow& window = desc.window;
        window.GetSize(width, height);
        glm::mat4 view_projection = desc.camera.get_view_projection_matrix(width, height);

        //Texture Manager
        auto& tm = TextureManager::instance();

        //Light
        glm::vec3 light_position(3.f, 3.f, 3.f);
        glm::vec3 light_colour(1.0f, 1.f, 1.f);

        //Shader: Lit Directional
        s_Data.diffuse_shader->use();
        s_Data.diffuse_shader->setMat4("view_projection", view_projection);
        //cube's material properties
        s_Data.diffuse_shader->setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        s_Data.diffuse_shader->setFloat("material.shininess", 32.0f);
        //flat lighting
        glm::vec3 light_direction = glm::vec3(-0.2, -1.0, -0.3);
        s_Data.diffuse_shader->setVec3("light.direction", light_direction);
        s_Data.diffuse_shader->setVec3("light.ambient", glm::vec3(0.2, 0.2, 0.2));
        s_Data.diffuse_shader->setVec3("light.diffuse", glm::vec3(0.5, 0.5, 0.5)); // darken diffuse light a bit
        s_Data.diffuse_shader->setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));

        //Light Object
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, light_position);
        model2 = glm::scale(model2, glm::vec3(0.2f)); // a smaller object

        glm::vec3 ambient_colour_1 = glm::vec3(1.0, 1.0, 1.0);
        glm::vec3 diffuse_colour_1 = ambient_colour_1 * 0.2f;
        s_Data.diffuse_shader->setVec3("material.diffuse", ambient_colour_1);
        s_Data.diffuse_shader->setVec3("material.ambient", diffuse_colour_1);
        s_Data.diffuse_shader->setMat4("model", model2);
        state.cubes[0]->model->draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);

        //Draw lit cubes
        tm.bind_texture("GENERATED/white_texture");
        for (int i = 0; i < state.cubes.size(); i++)
        {
            std::shared_ptr<FGObject> cube = state.cubes[i];

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cube->transform.Position);
            model = glm::scale(model, cube->transform.Scale);
            glm::vec3 ambient_colour_2 = glm::vec3(1.0, 0.0, 0.0);
            glm::vec3 diffuse_colour_2 = ambient_colour_2 * 0.2f;
            s_Data.diffuse_shader->setVec3("material.diffuse", ambient_colour_2);
            s_Data.diffuse_shader->setVec3("material.ambient", diffuse_colour_2);
            s_Data.diffuse_shader->setMat4("model", model);
            cube->model->draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);
        }
        tm.unbind_texture("GENERATED/white_texture");

        //Terrain
        {
            glm::mat4 model4 = glm::mat4(1.0f);
            model4 = glm::translate(model4, state.terrain->transform.Position);
            model4 = glm::scale(model4, state.terrain->transform.Scale);
            glm::vec3 ambient_colour_3 = glm::vec3(0.3, 0.3, 0.3);
            glm::vec3 diffuse_colour_3 = ambient_colour_3 * 0.2f;
            s_Data.diffuse_shader->setVec3("material.diffuse", ambient_colour_3);
            s_Data.diffuse_shader->setVec3("material.ambient", diffuse_colour_3);
            s_Data.diffuse_shader->setMat4("model", model4);
            state.terrain->model->draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);
        }

        //Player Object
        {
            std::shared_ptr<FGObject> player = state.player;

            glm::mat4 render_info = glm::mat4(1.0f);
            render_info = glm::translate(render_info, state.player->transform.Position);
            render_info = glm::scale(render_info, state.player->transform.Scale);
            glm::vec3 ambient_colour = glm::vec3(0.3, 1.0, 1.0);
            glm::vec3 diffuse_colour = ambient_colour * 0.2f;
            s_Data.diffuse_shader->setVec3("material.diffuse", ambient_colour);
            s_Data.diffuse_shader->setVec3("material.ambient", diffuse_colour);
            s_Data.diffuse_shader->setMat4("model", render_info);
            player->model->draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);
        }

        //Lizard Model
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, lizard.Position);
        //model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        //s_Data.diffuse_shader->setMat4("model", model);
        //FGModel& lizard_model = lizard.model;
        //lizard_model.Draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);

        ImGui::Begin("Renderer Profiler");
        ImGui::Text("Draw Calls: %i", s_Data.stats.DrawCalls);
        ImGui::End();
        s_Data.stats.DrawCalls = 0;
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

        std::string glsl_version = "";
#ifdef __APPLE__
        // GL 3.2 Core + GLSL 150
        glsl_version = "#version 150";
        SDL_GL_SetAttribute( // required on Mac OS
            SDL_GL_CONTEXT_FLAGS,
            SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
        );
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#elif __linux__
        // GL 3.2 Core + GLSL 150
        glsl_version = "#version 150";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#elif _WIN32
        // GL 3.0 + GLSL 130
        glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif

        // setup platform/renderer bindings
        ImGui_ImplSDL2_InitForOpenGL(window.GetHandle(), gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());

        //configure opengl state
        RenderCommand::init();
    }
}
