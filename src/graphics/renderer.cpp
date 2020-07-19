#include "graphics/renderer.h"

#include "graphics/shapes.h"
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

        Ref<Shader> diffuse_shader;

        renderer::Statistics stats;
    };
    static RenderData s_Data;

    void renderer::init_models_and_shaders(std::vector<std::reference_wrapper<FGTransform>>& models)
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // uncomment this call to draw in wireframe polygons.

        //Diffuse Shader
        s_Data.diffuse_shader = std::make_unique<Shader>
            ("assets/shaders/lit_directional.vert", "assets/shaders/lit_directional.frag");

        ////Lit Object Shader
        //s_Data.lit_object_shader = std::make_unique<Shader>
        //    ("assets/shaders/lit_directional.vert", "assets/shaders/lit_directional.frag");
    }

    void renderer::draw_pass(draw_scene_desc& desc)
    {
        render_command::SetClearColor(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        render_command::Clear();

        int width, height = 0;
        game_window& window = desc.window;
        window.GetSize(width, height);
        glm::mat4 view_projection = desc.camera.GetViewProjectionMatrix(width, height);

        //Texture Manager
        auto& tm = texture_manager::instance();

        //Light
        glm::vec3 light_position(3.f, 3.f, 3.f);
        glm::vec3 light_colour(1.0f, 1.f, 1.f);
        //Models
        FGTransform& lizard = desc.transforms[0];
        FGTransform& cube = desc.transforms[1];

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
        FGModel& cube_model = cube.model;
        cube_model.Draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);

        //Draw lit cube
        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(1.0f, 0.0f, 0.0f));
        model3 = glm::scale(model3, glm::vec3(1.0f, 1.0f, 1.0f));
        glm::vec3 ambient_colour_2 = glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 diffuse_colour_2 = ambient_colour_1 * 0.2f;
        s_Data.diffuse_shader->setVec3("material.diffuse", ambient_colour_2);
        s_Data.diffuse_shader->setVec3("material.ambient", diffuse_colour_2);
        s_Data.diffuse_shader->setMat4("model", model3);
        tm.bind_texture("white_texture");
        cube_model.Draw(*s_Data.diffuse_shader, s_Data.stats.DrawCalls);
        tm.unbind_texture("white_texture");

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

    void renderer::new_frame(SDL_Window* window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);

        ImGui::SetCurrentContext(_imgui);
        ImGui::NewFrame();
    }

    void renderer::end_frame(SDL_Window* window)
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

    void renderer::shutdown()
    {
        ImGui::SetCurrentContext(_imgui);

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(get_gl_context());
    }

    //void renderer::render_square(Shader& shader)
    //{
    //    //get current position from Box2D
    //    //b2Vec2 pos = physicsBody->GetPosition();
    //    //float angle = physicsBody->GetAngle();
    //    glm::vec3 glSize = glm::vec3(/*GetSizeForRenderer()*/ glm::vec2(1.0, 1.0), 1.0f);
    //    glm::vec3 glPos = glm::vec3(/*pos.x, pos.y*/ 0.0f, 0.0f, 0.0f);
    //    glm::mat4 idxMatrix = glm::mat4(1.0f);
    //    idxMatrix = glm::translate(idxMatrix, glPos) * glm::scale(idxMatrix, { glSize });
    //    shader.setMat4("u_Transform", idxMatrix);
    //    shader.setVec4("u_Color", { /*GetColor()*/ glm::vec4(1.0, 0.0, 0.0, 1.0) });
    //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //}

    void renderer::init_opengl_and_imgui(const game_window& window)
    {
        //OpenGL
        gl_context = SDL_GL_CreateContext(window.GetHandle());
        SDL_GL_MakeCurrent(window.GetHandle(), gl_context);

        int width, height;
        window.GetSize(width, height);
        render_command::SetViewport(0, 0, width, height);

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

        render_command::Init(); //configure opengl state
    }
}

/* LIGHTING CODE

// be sure to activate shader when setting uniforms/drawing objects
//lightingShader.use();
//lightingShader.setVec3("light.position", lightPos);
//lightingShader.setVec3("viewPos", camera.Position);

//// light properties
//glm::vec3 lightColor;
//lightColor.x = sin(time * 2.0f);
//lightColor.y = sin(time * 0.7f);
//lightColor.z = sin(time * 1.3f);
//glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f); // decrease the influence
//glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f); // low influence
//lightingShader.setVec3("light.ambient", ambientColor);
//lightingShader.setVec3("light.diffuse", diffuseColor);
//lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

//// material properties
//lightingShader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
//lightingShader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
//lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f); // specular lighting doesn't have full effect on this object's material
//lightingShader.setFloat("material.shininess", 32.0f);
//// render the cube
//glBindVertexArray(cubeVAO);
//glDrawArrays(GL_TRIANGLES, 0, 36);

//// also draw the lamp object
//lampShader.use();
//lampShader.setMat4("projection", projection);
//lampShader.setMat4("view", view);
//model = glm::mat4(1.0f);
//model = glm::translate(model, lightPos);
//model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
//lampShader.setMat4("model", model);
//lampShader.setVec3("color", diffuseColor);

//glBindVertexArray(lightVAO);
//glDrawArrays(GL_TRIANGLES, 0, 36);

*/

/* Examples
//camera rotate around point
//float radius = 10.0f;
//float camX = sin(glfwGetTime()) * radius;
//float camZ = cos(glfwGetTime()) * radius;
*/

//s_Data.AllVertexArray = vertex_array::Create();
//s_Data.AllVertexBuffer = vertex_buffer::Create(s_Data.MaxVertices);
//s_Data.AllVertexBuffer->SetLayout
//({
//    { shader_data_type::Float3, "aPos" },
//    { shader_data_type::Float3, "aNormal" },
//    { shader_data_type::Float2, "aTexCoords" },
//    });
//s_Data.AllVertexArray->AddVertexBuffer(s_Data.AllVertexBuffer);
//s_Data.CubeVertexBufferBase = new Vertex[s_Data.MaxVertices];
