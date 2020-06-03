#include "renderer.h"

#include "window/game_window.h"
#include "graphics/shapes.h"

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
    void renderer::init_opengl(const game_window* window)
    {
        //OpenGL
        gl_context = SDL_GL_CreateContext(window->GetHandle());
        SDL_GL_MakeCurrent(window->GetHandle(), gl_context);

        int width, height;
        window->GetSize(width, height);
        configure_view(render_pass::Main, width, height);

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

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, // left  
             0.5f, -0.5f, 0.0f, // right 
             0.0f,  0.5f, 0.0f  // top   
        };

        unsigned int VBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BASIC_SQUARE), BASIC_SQUARE, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(BASIC_SQUARE_INDICIES), BASIC_SQUARE_INDICIES, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // uncomment this call to draw in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        //Load texture
        texId = loadTexture("res/textures/Misc/orangecurvedwallsofsandstone.jpg");

        // build and compile our shader program
        // ------------------------------------
        flatColorShader = std::make_unique<Shader>("res/shaders/flat_color.vert", "res/shaders/flat_color.frag");
        //flatColorShader->use();
        //flatColorShader->setInt("u_Texture", texId);
    }

    renderer::~renderer()
    {
        //_shaderManager.reset();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
    }

    void renderer::configure_view(graphics::render_pass view_id, uint16_t width, uint16_t height) const
    {
        glViewport(0, 0, width, height);
    }

    void renderer::new_frame(SDL_Window* window)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
    }

    void renderer::draw_scene(const draw_scene_desc& desc) const
    {
        //// Reflection Pass
        //{
        //	auto section = drawDesc.profiler.BeginScoped(Profiler::Stage::ReflectionPass);
        //	if (drawDesc.drawWater)
        //	{
        //		DrawSceneDesc drawPassDesc = drawDesc;

        //		auto& frameBuffer = drawDesc.water.GetFrameBuffer();
        //		auto reflectionCamera = drawDesc.camera->Reflect(drawDesc.water.GetReflectionPlane());

        //		drawPassDesc.viewId = graphics::RenderPass::Reflection;
        //		drawPassDesc.camera = reflectionCamera.get();
        //		drawPassDesc.frameBuffer = &frameBuffer;
        //		drawPassDesc.drawWater = false;
        //		drawPassDesc.drawDebugCross = false;
        //		drawPassDesc.drawBoundingBoxes = false;
        //		drawPassDesc.cullBack = true;
        //		DrawPass(meshPack, drawPassDesc);
        //	}
        //}

        //// Main Draw Pass
        //{
        //	auto section = drawDesc.profiler.BeginScoped(Profiler::Stage::MainPass);
        //	DrawPass(meshPack, drawDesc);
        //}

        draw_pass(desc);
    }

    void renderer::draw_pass(const draw_scene_desc& desc) const
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        //bind texures
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, texId);

        // camera
        //glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 view = camera.GetViewMatrix();
        //glm::mat4 view_projection = projection * view;

        //bind texures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);

        // activate shader
        flatColorShader->use();

        // pass transformation matrices to the shader
        // note: currently we set the projection matrix each frame, but since the projection
        // matrix rarely changes it's often best practice to set it outside the main loop only once.
        //flatColorShader->setMat4("u_ViewProjection", view_projection); 

        // draw our first triangle
        glm::vec3 glSize = glm::vec3(/*GetSizeForRenderer()*/ glm::vec2(1.0, 1.0), 1.0f);
        glm::vec3 glPos = glm::vec3(/*pos.x, pos.y*/ 0.0f, 0.0f, -1.0f);
        glm::mat4 idxMatrix = glm::mat4(1.0f);
        idxMatrix = glm::translate(idxMatrix, glPos) * glm::scale(idxMatrix, { glSize });
        //flatColorShader->setMat4("u_Transform", idxMatrix);
        flatColorShader->setVec4("ourColor", { /*GetColor()*/ glm::vec4(1.0, 0.0, 0.0, 1.0) });

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

        // activate shader

        // pass transformation matrices to the shader
        //flatColorShader->setMat4("u_ViewProjection", view_projection);

        //the end
    }

    void renderer::render_at_position(Shader* shader)
    {
        //get current position from Box2D
        //b2Vec2 pos = physicsBody->GetPosition();
        //float angle = physicsBody->GetAngle();

        glm::vec3 glSize = glm::vec3(/*GetSizeForRenderer()*/ glm::vec2(1.0, 1.0), 1.0f);
        glm::vec3 glPos = glm::vec3(/*pos.x, pos.y*/ 0.0f, 0.0f, 0.0f);

        glm::mat4 idxMatrix = glm::mat4(1.0f);
        idxMatrix = glm::translate(idxMatrix, glPos) * glm::scale(idxMatrix, { glSize });

        shader->setMat4("u_Transform", idxMatrix);
        shader->setVec4("u_Color", { /*GetColor()*/ glm::vec4(1.0, 0.0, 0.0, 1.0) });

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void renderer::end_frame(SDL_Window* window)
    {
        ImGui::SetCurrentContext(_imgui);
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplOpenGL3_RenderDrawData(draw_data);

        SDL_GL_SwapWindow(window);
    }

    // utility function for loading a 2D texture from file
    // ---------------------------------------------------
    unsigned int renderer::loadTexture(char const* path)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
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