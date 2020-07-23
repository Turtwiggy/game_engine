#include "graphics/renderer.h"

#include "data/shapes.h"
#include "graphics/render_command.h"
#include "window/game_window.h"
#include "3d/fg_texture.hpp"

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

        Shader hdr_shader;
        Shader lit_directional_tex_shader;

        unsigned int wood_texture;
        unsigned int hdr_fbo;
        unsigned int colour_buffer;
        std::vector<glm::vec3> light_positions;
        std::vector<glm::vec3> light_colours;

        Renderer::Statistics stats;
    };
    static RenderData s_Data;

    unsigned int Renderer::hdr_fbo()
    {
        // configure floating point framebuffer
        // ------------------------------------
        unsigned int hdrFBO;
        glGenFramebuffers(1, &hdrFBO);
        return hdrFBO;
    }

    unsigned int Renderer::hdr_colour_buffer(int width, int height, unsigned int hdr_fbo)
    {
        // create floating point color buffer
        unsigned int colorBuffer;
        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // create depth buffer (renderbuffer)
        unsigned int rboDepth;
        glGenRenderbuffers(1, &rboDepth);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        // attach buffers
        glBindFramebuffer(GL_FRAMEBUFFER, hdr_fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return colorBuffer;
    }

    void Renderer::init_renderer(int screen_width, int screen_height)
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // uncomment this call to draw in wireframe polygons.

        s_Data.hdr_shader = Shader("assets/shaders/hdr.vert", "assets/shaders/hdr.frag");
        s_Data.lit_directional_tex_shader = Shader("assets/shaders/lit_directional_tex.vert", "assets/shaders/lit_directional_tex.frag");

        //Enable Multi Sampling
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        glEnable(GL_MULTISAMPLE);

        //Enable Faceculling
        //glEnable(GL_CULL_FACE);
        //glDepthFunc(GL_LESS);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Enable depth testing
        glEnable(GL_DEPTH_TEST);

        //From now on your rendered images will be gamma corrected and as this is done by the hardware it is completely free.
        //Something you should keep in mind with this approach (and the other approach) is that gamma correction (also) transforms the colors from linear space to non-linear space so it is very important you only do gamma correction at the last and final step. 
        //If you gamma-correct your colors before the final output, all subsequent operations on those colors will operate on incorrect values. 
        //For instance, if you use multiple framebuffers you probably want intermediate results passed in between framebuffers to remain in linear-space and only have the last framebuffer apply gamma correction before being sent to the monitor.
        glEnable(GL_FRAMEBUFFER_SRGB);

        s_Data.hdr_fbo = hdr_fbo();
        s_Data.colour_buffer = hdr_colour_buffer(screen_width, screen_height, s_Data.hdr_fbo);

        // lighting info
        // -------------
        // positions
        std::vector<glm::vec3> lightPositions;
        lightPositions.push_back(glm::vec3(0.0f, 0.0f, 49.5f)); // back light
        lightPositions.push_back(glm::vec3(-1.4f, -1.9f, 9.0f));
        lightPositions.push_back(glm::vec3(0.0f, -1.8f, 4.0f));
        lightPositions.push_back(glm::vec3(0.8f, -1.7f, 6.0f));
        s_Data.light_positions = lightPositions;
        // colors
        std::vector<glm::vec3> lightColors;
        lightColors.push_back(glm::vec3(100.0f, 100.0f, 100.0f));
        lightColors.push_back(glm::vec3(0.1f, 0.0f, 0.0f));
        lightColors.push_back(glm::vec3(0.0f, 0.0f, .2f));
        lightColors.push_back(glm::vec3(0.0f, 0.1f, 0.0f));
        s_Data.light_colours = lightColors;

        s_Data.hdr_shader.use();
        s_Data.hdr_shader.setInt("hdrBuffer", 0);
        s_Data.wood_texture = TextureFromFile("BambooWall_1K_albedo.jpg", "assets/textures/Bamboo", true);
    }

    void Renderer::draw_pass(draw_scene_desc& desc, GameState state)
    {
        RenderCommand::set_clear_colour(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        RenderCommand::clear();

        int width, height = 0;
        GameWindow& window = desc.window;
        window.GetSize(width, height);
        glm::mat4 view_projection = desc.camera.get_view_projection_matrix(width, height);

        //Light
        //glm::vec3 light_position(3.f, 3.f, 3.f);
        //glm::vec3 light_colour(1.0f, 1.f, 1.f);
        ////Shader: Lit Directional
        //s_Data.lit_directional_shader->use();
        //s_Data.lit_directional_shader->setMat4("view_projection", view_projection);
        ////cube's material properties
        //s_Data.lit_directional_shader->setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        //s_Data.lit_directional_shader->setFloat("material.shininess", 32.0f);
        ////flat lighting
        //glm::vec3 light_direction = glm::vec3(-0.2, -1.0, -0.3);
        //s_Data.lit_directional_shader->setVec3("light.direction", light_direction);
        //s_Data.lit_directional_shader->setVec3("light.ambient", glm::vec3(0.2, 0.2, 0.2));
        //s_Data.lit_directional_shader->setVec3("light.diffuse", glm::vec3(0.5, 0.5, 0.5)); // darken diffuse light a bit
        //s_Data.lit_directional_shader->setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));
        ////Light Object
        //glm::mat4 model2 = glm::mat4(1.0f);
        //model2 = glm::translate(model2, light_position);
        //model2 = glm::scale(model2, glm::vec3(0.2f)); // a smaller object

        //glm::vec3 ambient_colour_1 = glm::vec3(1.0, 1.0, 1.0);
        //glm::vec3 diffuse_colour_1 = ambient_colour_1 * 0.2f;
        ////s_Data.lit_directional_shader->setVec3("material.diffuse", ambient_colour_1);
        ////s_Data.lit_directional_shader->setVec3("material.ambient", diffuse_colour_1);
        //s_Data.lit_directional_shader->setMat4("model", model2);
        //state.cubes[0]->model->draw(*s_Data.lit_directional_shader, s_Data.stats.DrawCalls);

        // 1. render scene into floating point framebuffer
        // -----------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, s_Data.hdr_fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Cornell
        {
            s_Data.lit_directional_tex_shader.use();
            s_Data.lit_directional_tex_shader.setMat4("view_projection", view_projection);
            //cube's material properties
            s_Data.lit_directional_tex_shader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
            s_Data.lit_directional_tex_shader.setFloat("material.shininess", 32.0f);
            //flat lighting
            glm::vec3 light_direction = glm::vec3(-0.2, -1.0, -0.3);
            s_Data.lit_directional_tex_shader.setVec3("light.direction", light_direction);
            s_Data.lit_directional_tex_shader.setVec3("light.ambient", glm::vec3(0.2, 0.2, 0.2));
            s_Data.lit_directional_tex_shader.setVec3("light.diffuse", glm::vec3(0.5, 0.5, 0.5)); // darken diffuse light a bit
            s_Data.lit_directional_tex_shader.setVec3("light.specular", glm::vec3(1.0, 1.0, 1.0));

            //Positioning
            std::shared_ptr<FGObject> object = state.cornel_box;
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            s_Data.lit_directional_tex_shader.setMat4("model", model);
            object->model->draw(s_Data.lit_directional_tex_shader, s_Data.stats.DrawCalls);
        }

        // 2. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        s_Data.hdr_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, s_Data.colour_buffer);
        s_Data.hdr_shader.setInt("hdr", desc.hdr);
        s_Data.hdr_shader.setFloat("exposure", desc.exposure);
        renderQuad();

        //std::cout << "hdr: " << (true ? "on" : "off") << "| exposure: " << 1.0f << std::endl;

        ////Draw lit cubes
        //for (int i = 0; i < state.cubes.size(); i++)
        //{
        //    std::shared_ptr<FGObject> cube = state.cubes[i];
        //    glm::mat4 model = glm::mat4(1.0f);
        //    model = glm::translate(model, cube->transform.Position);
        //    model = glm::scale(model, cube->transform.Scale);
        //    glm::vec3 ambient_colour_2 = glm::vec3(1.0, 0.0, 0.0);
        //    glm::vec3 diffuse_colour_2 = ambient_colour_2 * 0.2f;
        //    s_Data.lit_directional_shader->setVec3("material.diffuse", ambient_colour_2);
        //    s_Data.lit_directional_shader->setVec3("material.ambient", diffuse_colour_2);
        //    s_Data.lit_directional_shader->setMat4("model", model);
        //    cube->model->draw(*s_Data.lit_directional_shader, s_Data.stats.DrawCalls);
        //}


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
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#endif

        // setup platform/renderer bindings
        ImGui_ImplSDL2_InitForOpenGL(window.GetHandle(), gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());

        //configure opengl state
        RenderCommand::init();
    }

    // renderQuad() renders a 1x1 XY quad in NDC
    // -----------------------------------------
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    void Renderer::renderQuad()
    {
        if (quadVAO == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        }
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
}
