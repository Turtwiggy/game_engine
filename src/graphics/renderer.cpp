#include "graphics/renderer.h"

#include "graphics/shapes.h"
#include "graphics/render_command.h"
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

#include <filesystem>

using namespace fightinggame;
using namespace fightinggame::graphics;

namespace fightinggame
{
    struct RenderData
    {
        // TODO: RenderCaps
        static const uint32_t MaxModels = 20;
        static const uint32_t MaxTextureSlots = 32;
        static const uint32_t MaxVertices = 10000;
        static const uint32_t MaxIndicies = 10000;

        Ref<Shader> lizard_shader;
        Ref<Shader> cube_shader;
        Ref<texture2D> white_texture;

        std::array<Ref<texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        renderer::Statistics stats;
    };

    static RenderData s_Data;

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

    void renderer::init_models_and_shaders(std::vector<std::reference_wrapper<Model>>& models)
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // uncomment this call to draw in wireframe polygons.

        //// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
        //stbi_set_flip_vertically_on_load(true);

        //Ref<index_buffer> ib = index_buffer::Create(/* MODEL INDICIES */, s_Data.MaxIndicies);
        //s_Data.AllVertexArray->SetIndexBuffer(ib);

        //create a texture
        s_Data.white_texture = texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.white_texture->set_data(&whiteTextureData, sizeof(uint32_t));

        // build and compile our shader program
        // ------------------------------------
        //Lizard
        s_Data.lizard_shader = std::make_unique<Shader>
            ("res/shaders/diffuse.vert", "res/shaders/diffuse.frag");
        s_Data.lizard_shader->use();
        //s_Data.lizard_shader->SetIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        //Cube 
        cube_pos = glm::vec3(1.f, 0.f, 0.f);
        s_Data.cube_shader = std::make_unique<Shader>
            ("res/shaders/diffuse.vert", "res/shaders/diffuse.frag");
        s_Data.cube_shader->use();

        //Load texture
        s_Data.TextureSlots[0] = s_Data.white_texture;
        s_Data.TextureSlotIndex = 1;

        //Models
        Model& lizard = models[0];
        std::vector<Ref<texture2D>> lizard_textures = lizard.get_textures();
        for (auto tex_index = 0; tex_index < lizard_textures.size(); tex_index++)
        {
            std::cout << "adding texture: " << lizard_textures[tex_index]->get_path() << std::endl;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = lizard_textures[tex_index];
            s_Data.TextureSlotIndex += 1;
        }

        Model& cube = models[1];

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        for (auto tex_index = 0; tex_index < s_Data.TextureSlots.size(); tex_index++)
        {
            Ref<texture2D> texture = s_Data.TextureSlots[tex_index];
            if (texture != nullptr)
                std::cout << "texture (id) " << texture->get_renderer_id() << " path: " << texture->get_path() << std::endl;
        }
    }

    void renderer::draw_pass(draw_scene_desc& desc)
    {
        render_command::SetClearColor(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        render_command::Clear();

        ImGui::Begin("Hello from the renderer");
        ImGui::Button("Hello button");
        ImGui::End();

        glm::mat4 projection = glm::perspective(glm::radians(desc.camera.Zoom), (float)desc.width / (float)desc.height, 0.1f, 100.0f);
        glm::mat4 view = desc.camera.GetViewMatrix();
        glm::mat4 view_projection = projection * view;

        //Begin Scene
        //s_Data.AllIndexCount = 0;
        //s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;
        s_Data.TextureSlotIndex = 1;

        //End Scene
        //uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CubeVertexBufferPtr - (uint8_t*)s_Data.CubeVertexBufferBase);
        //s_Data.AllVertexBuffer->SetData(s_Data.CubeVertexBufferBase, dataSize);

        //shaders
        s_Data.lizard_shader->use();
        s_Data.cube_shader->use();

        s_Data.lizard_shader->setMat4("projection", projection);
        s_Data.lizard_shader->setMat4("view", view);
        s_Data.cube_shader->setMat4("projection", projection);
        s_Data.cube_shader->setMat4("view", view);

        //Lizard Stuff
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        s_Data.lizard_shader->setMat4("model", model);
        Model& lizard = desc.models[0];

        s_Data.lizard_shader->setInt("texture_diffuse1", s_Data.TextureSlots[2]->get_renderer_id());

        //Cube Stuff
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, cube_pos); // translate it down so it's at the center of the scene
        model2 = glm::scale(model2, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        s_Data.cube_shader->setMat4("model", model2);
        Model& cube = desc.models[1];
        //s_Data.cube_shader->setInt("texture_diffuse1", s_Data.TextureSlots[1]->get_renderer_id());

        // Bind textures
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
        {
            Ref<texture2D> texture = s_Data.TextureSlots[i];
            if (texture == nullptr)
                continue;

            std::cout << "binding: " << s_Data.TextureSlots[i]->get_path() << "to id: " << i << std::endl;
            s_Data.TextureSlots[i]->bind(i);
        }

        lizard.Draw(*s_Data.lizard_shader);
        cube.Draw(*s_Data.cube_shader);

        ////flatColorShader->setVec4("u_Color", { /*GetColor()*/ glm::vec4(1.0, 0.0, 0.0, 1.0) });
    }

    void flush()
    {
        //if (s_Data.AllIndexCount == 0)
        //    return; // Nothing to draw

        // Bind textures
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
            s_Data.TextureSlots[i]->bind(i);

        //render_command::DrawIndexed(s_Data.AllVertexArray, s_Data.AllIndexCount);
        s_Data.stats.DrawCalls++;
    }

    void flush_and_reset()
    {
        //uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
        //s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

        flush();

        //s_Data.QuadIndexCount = 0;
        //s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
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

    void renderer::render_square(Shader& shader)
    {
        //get current position from Box2D
        //b2Vec2 pos = physicsBody->GetPosition();
        //float angle = physicsBody->GetAngle();

        glm::vec3 glSize = glm::vec3(/*GetSizeForRenderer()*/ glm::vec2(1.0, 1.0), 1.0f);
        glm::vec3 glPos = glm::vec3(/*pos.x, pos.y*/ 0.0f, 0.0f, 0.0f);

        glm::mat4 idxMatrix = glm::mat4(1.0f);
        idxMatrix = glm::translate(idxMatrix, glPos) * glm::scale(idxMatrix, { glSize });

        shader.setMat4("u_Transform", idxMatrix);
        shader.setVec4("u_Color", { /*GetColor()*/ glm::vec4(1.0, 0.0, 0.0, 1.0) });

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
