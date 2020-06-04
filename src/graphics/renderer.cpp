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

#include "imgui.h"
#include <examples/imgui_impl_sdl.h>
#include <examples/imgui_impl_opengl3.h>

using namespace fightinggame;
using namespace fightinggame::graphics;

namespace fightinggame
{
    struct CubeVertex
    {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
    };

    struct RenderData
    {
        static const uint32_t MaxCubes = 20000;
        static const uint32_t MaxVertices = MaxCubes * 30;
        static const uint32_t MaxIndices = MaxCubes * 36;
        static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

        Ref<vertex_array> CubeVertexArray;
        Ref<vertex_buffer> CubeVertexBuffer;
        Ref<Shader> TextureShader;
        //Ref<Texture2D> Texture;

        uint32_t CubeIndexCount = 0;
        CubeVertex* CubeVertexBufferBase = nullptr;
        CubeVertex* CubeVertexBufferPtr = nullptr;

        //std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        //uint32_t TextureSlotIndex = 1; // 0 = white texture

        //glm::vec4 QuadVertexPositions[4];

        renderer::Statistics Stats;
    };
    static RenderData s_Data;


    void renderer::init_opengl(const game_window* window)
    {
        //OpenGL
        gl_context = SDL_GL_CreateContext(window->GetHandle());
        SDL_GL_MakeCurrent(window->GetHandle(), gl_context);

        int width, height;
        window->GetSize(width, height);
        render_command::SetViewport(0, 0, width, height);

        //Setup ImGui
        IMGUI_CHECKVERSION();
        _imgui = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        //style.ScaleAllSizes(scale);
        //io.FontGlobalScale = scale;

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
        ImGui_ImplSDL2_InitForOpenGL(window->GetHandle(), gl_context);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());
    }

    renderer::renderer(const game_window* window, bool vsync)
    {
        init_opengl(window);
        render_command::Init(); //configure opengl state

        //Load texture
        //texID = loadTexture("res/textures/Bamboo/BambooWall_1K_albedo.jpg");

        // uncomment this call to draw in wireframe polygons.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        s_Data.CubeVertexArray = vertex_array::Create();
        s_Data.CubeVertexBuffer = vertex_buffer::Create(s_Data.MaxVertices * sizeof(BASIC_CUBE));
        s_Data.CubeVertexBuffer->SetLayout
        ({
            { shader_data_type::Float3, "aPos" },
            { shader_data_type::Float2, "aTexCoord" },
        });
        s_Data.CubeVertexArray->AddVertexBuffer(s_Data.CubeVertexBuffer);

        s_Data.CubeVertexBufferBase = new CubeVertex[s_Data.MaxVertices];

        //todo sort out indices 

        // build and compile our shader program
        // ------------------------------------
        shader = std::make_unique<Shader>
            ("res/shaders/textured_transform.vert", "res/shaders/textured_transform.frag");
        shader->use();
        //shader->setInt("texture1", texID);
    }

    renderer::~renderer()
    {
        //_shaderManager.reset();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
    }

    void renderer::draw_pass(const draw_scene_desc& desc)
    {
        render_command::SetClearColor(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        render_command::Clear();

        ImGui::Begin("Hello Second");
        ImGui::Button("Hello Second button");
        ImGui::End();

        shader->use();
        glm::mat4 projection = glm::perspective(glm::radians(desc.camera->Zoom), (float)desc.width / (float)desc.height, 0.1f, 100.0f);
        glm::mat4 view = desc.camera->GetViewMatrix();
        glm::mat4 view_projection = projection * view;
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);

        //bind texures
        //uint32_t slot = 0;
        //glBindTextureUnit(slot, texID);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        shader->setMat4("model", model);
        desc.main_character->Draw(*shader);

        //s_Data.CubeIndexCount = 0;
        //s_Data.CubeVertexBufferPtr = s_Data.CubeVertexBufferBase;
        ////s_Data.TextureSlotIndex = 1;


        ////draw some cubes
        //for (unsigned int i = 0; i < 10; i++)
        //glm::vec3 size = glm::vec3(1.f, 1.f, 1.f);
        //glm::vec3 pos = glm::vec3(0., 0., 0.);
        //render_cube(pos, size);

        //flush
        //if (s_Data.CubeIndexCount == 0) {
        //    printf("nothing to draw");
        //    return; // Nothing to draw
        //}

        //render_command::DrawIndexed(s_Data.CubeVertexArray, s_Data.CubeIndexCount);
        //s_Data.Stats.DrawCalls++;

        //// pass transformation matrices to the shader
        //// note: currently we set the projection matrix each frame, but since the projection
        //// matrix rarely changes it's often best practice to set it outside the main loop only once.
        ////flatColorShader->setMat4("u_ViewProjection", view_projection); 

        ////glm::vec3 glSize = glm::vec3(/*GetSizeForRenderer()*/ glm::vec2(1.0, 1.0), 1.0f);
        ////glm::vec3 glPos = glm::vec3(/*pos.x, pos.y*/ 0.0f, 0.0f, -1.0f);
        ////glm::mat4 idxMatrix = glm::mat4(1.0f);
        ////idxMatrix = glm::translate(idxMatrix, glPos) * glm::scale(idxMatrix, { glSize });
        ////flatColorShader->setVec4("u_Color", { /*GetColor()*/ glm::vec4(1.0, 0.0, 0.0, 1.0) });
    }


    void renderer::render_cube(glm::vec3& position, glm::vec3& size)
    {
        constexpr size_t cubeVertexCount = 30;
        const float textureIndex = 0.0f; //First texture
        const float tilingFactor = 1.0f;

        //if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        //    FlushAndReset();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < cubeVertexCount; i++)
        {
            s_Data.CubeVertexBufferPtr->Position = transform * glm::vec4(1.0, 1.0, 1.0, 1.0); /* * BASIC_CUBE[i] */
            //s_Data.CubeVertexBufferPtr->Color = color;
            //s_Data.CubeVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.CubeVertexBufferPtr->TexIndex = textureIndex;
            s_Data.CubeVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.CubeVertexBufferPtr++;
        }

        // calculate the model matrix for each object and pass it to shader before drawing
        //glm::mat4 model = glm::mat4(1.0f);
        //model = glm::translate(model, cubePositions[i]);
        //float angle = 20.0f * i;
        //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        //flatColorShader->setMat4("model", model);

        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //s_Data.CubeIndexCount += 30;
        s_Data.Stats.QuadCount++;

        return;
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
        ImGui::SetCurrentContext(_imgui);
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplOpenGL3_RenderDrawData(draw_data);

        SDL_GL_SwapWindow(window);
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

//// view/projection transformations
//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//glm::mat4 view = camera.GetViewMatrix();
//lightingShader.setMat4("projection", projection);
//lightingShader.setMat4("view", view);

//// world transformation
//glm::mat4 model = glm::mat4(1.0f);
//lightingShader.setMat4("model", model);

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



/* TEXTURE Examples

// bind textures on corresponding texture units
//glActiveTexture(GL_TEXTURE0);
//glBindTexture(GL_TEXTURE_2D, texture1);

// activate shader
//ourShader.use();

// create transformations
//glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//transform = glm::translate(transform, glm::vec3(1.0f, 0.0f, 0.0f));
//transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
//unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

//More transformations
//glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
// //retrieve the matrix uniform locations
//unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
// //pass them to the shaders
//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

//camera rotate around point
//float radius = 10.0f;
//float camX = sin(glfwGetTime()) * radius;
//float camZ = cos(glfwGetTime()) * radius;

// pass projection matrix to shader (note that in this case it could change every frame)
//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//ourShader.setMat4("projection", projection);

// camera/view transformation
//glm::mat4 view = camera.GetViewMatrix();
//ourShader.setMat4("view", view);

////render container
//glBindVertexArray(cubeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
//for (unsigned int i = 0; i < 10; i++)
//{
//	glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//	model = glm::translate(model, cubePositions[i]);
//	float angle = 20.0f * i;
//	model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
//	ourShader.setMat4("model", model);

//	glDrawArrays(GL_TRIANGLES, 0, 36);
//}

*/