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
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";

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

        // build and compile our shader program
        // ------------------------------------
        // vertex shader
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        // check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            -0.5f, -0.5f, 0.0f, // left  
             0.5f, -0.5f, 0.0f, // right 
             0.0f,  0.5f, 0.0f  // top   
        };

        unsigned int VBO = 0;
        VAO = 0;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);


        // build and compile our shader program
        // ------------------------------------
        //flatColorShader = std::make_unique<Shader>("src/graphics/shaders/texture_transform.vs", "src/graphics/shaders/texture_transform.fs");

        // uncomment this call to draw in wireframe polygons.
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        //texId = loadTexture("res/textures/Misc/orangecurvedwallsofsandstone.jpg");
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

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // activate shader
        //flatColorShader->use();

        // pass transformation matrices to the shader
        //flatColorShader->setMat4("u_ViewProjection", view_projection);

        //the end
    }

    void renderer::end_frame(SDL_Window* window)
    {
        //ImGui::SetCurrentContext(_imgui);

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